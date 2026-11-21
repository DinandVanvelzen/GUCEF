/*
 *  AssetWorkbenchLib: the scene view: a DotScene file in a stage, its node tree for the browser, the selection highlighted, its properties for the inspector
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
#include <Ogre.h>
#include <OgreCodec.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#include "gucefGUI_CPropertyRowDescriptor.h"
#define GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#endif /* GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H ? */

#include "AssetWorkbenchLib_CParticlePreview.h"
#include "AssetWorkbenchLib_CSceneView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const SelectionLines = "Selection";

/** A number with at most three decimals and without the zeros that end them */
CORE::CString
Number( Float64 value )
{
    return GUI::CPropertyRowDescriptor::NumberToText( floor( value * 1000.0 + 0.5 ) / 1000.0, -1 );
}

CORE::CString
VectorText( const Ogre::Vector3& v )
{
    return Number( v.x ) + ", " + Number( v.y ) + ", " + Number( v.z );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSceneView::CSceneView( CWorkbenchStage& stage )
    : m_stage( stage )
    , m_root( GUCEF_NULL )
    , m_tree( GUCEF_NEW CSceneTreeFileSystemAccess( "Scene" ) )
    , m_treePtr( m_tree )
    , m_groupName()
    , m_sceneName()
    , m_selectedPath( "/" )
    , m_nodeCount( 0 )
    , m_objectCount( 0 )
    , m_meshNames()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSceneView::~CSceneView()
{GUCEF_TRACE;

    Unload();
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessPtr
CSceneView::GetFileSystem( void ) const
{GUCEF_TRACE;

    return m_treePtr;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSceneView::GetObjectKind( const Ogre::MovableObject& object )
{GUCEF_TRACE;

    const Ogre::String& type = object.getMovableType();
    if ( "Entity" == type )
        return "entity";
    if ( "Light" == type )
        return "light";
    if ( "Camera" == type )
        return "camera";
    if ( "ParticleSystem" == type )
        return "particles";
    return "object";
}

/*-------------------------------------------------------------------------*/

void
CSceneView::DestroyTree( Ogre::SceneNode* node )
{GUCEF_TRACE;

    // the children first, then what is attached to the node
    const Ogre::Node::ChildNodeMap children = node->getChildren();
    Ogre::Node::ChildNodeMap::const_iterator c = children.begin();
    while ( c != children.end() )
    {
        DestroyTree( static_cast< Ogre::SceneNode* >( *c ) );
        ++c;
    }
    while ( node->numAttachedObjects() > 0 )
    {
        Ogre::MovableObject* object = node->getAttachedObject( (size_t) 0 );
        node->detachObject( object );
        m_stage.GetSceneManager()->destroyMovableObject( object );
    }
}

/*-------------------------------------------------------------------------*/

void
CSceneView::Unload( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_root && GUCEF_NULL != Ogre::Root::getSingletonPtr() && m_stage.IsInitialized() )
    {
        try
        {
            m_stage.ClearLineObjects();
            DestroyTree( m_root );
            m_stage.GetContentNode()->removeAndDestroyChild( m_root );
            for ( size_t i=0; i<m_meshNames.size(); ++i )
            {
                if ( !Ogre::MeshManager::getSingleton().resourceExists( m_meshNames[ i ].STL_String(), m_groupName.STL_String() ) )
                    continue;
                // the skeleton that a source model made goes with its mesh
                Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().getByName( m_meshNames[ i ].STL_String(), m_groupName.STL_String() );
                const Ogre::String skeletonName = mesh ? mesh->getSkeletonName() : Ogre::String();
                Ogre::MeshManager::getSingleton().remove( m_meshNames[ i ].STL_String(), m_groupName.STL_String() );
                if ( !skeletonName.empty() && Ogre::SkeletonManager::getSingleton().resourceExists( skeletonName, m_groupName.STL_String() ) )
                    Ogre::SkeletonManager::getSingleton().remove( skeletonName, m_groupName.STL_String() );
            }
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CSceneView: The scene cannot be taken out of the stage: " << e.getDescription() );
        }
    }
    m_root = GUCEF_NULL;
    m_meshNames.clear();
    m_nodeCount = 0;
    m_objectCount = 0;
    m_selectedPath = "/";
    m_tree->SetTree( CSceneTreeFileSystemAccess::TNodeVector() );
}

/*-------------------------------------------------------------------------*/

bool
CSceneView::IsLoaded( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_root;
}

/*-------------------------------------------------------------------------*/

Ogre::SceneNode*
CSceneView::GetRoot( void ) const
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

UInt32
CSceneView::GetNodeCount( void ) const
{GUCEF_TRACE;

    return m_nodeCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CSceneView::GetObjectCount( void ) const
{GUCEF_TRACE;

    return m_objectCount;
}

/*-------------------------------------------------------------------------*/

void
CSceneView::AddNodeToTree( Ogre::SceneNode* node, const CORE::CString& path, CSceneTreeFileSystemAccess::TNodeVector& nodes )
{GUCEF_TRACE;

    CSceneTreeFileSystemAccess::SNode entry;
    entry.path = path;
    for ( size_t i=0; i<node->numAttachedObjects(); ++i )
    {
        Ogre::MovableObject* object = node->getAttachedObject( i );
        CSceneTreeFileSystemAccess::SObject item;
        item.name = object->getName();
        item.kind = GetObjectKind( *object );
        entry.objects.push_back( item );
        ++m_objectCount;
        if ( "entity" == item.kind )
        {
            Ogre::Entity* entity = static_cast< Ogre::Entity* >( object );
            m_meshNames.push_back( entity->getMesh()->getName() );
        }
    }
    nodes.push_back( entry );
    if ( path != "/" )
        ++m_nodeCount;

    Ogre::Node::ChildNodeMap::const_iterator c = node->getChildren().begin();
    while ( c != node->getChildren().end() )
    {
        Ogre::SceneNode* child = static_cast< Ogre::SceneNode* >( *c );
        AddNodeToTree( child, CSceneTreeFileSystemAccess::JoinPath( path, child->getName() ), nodes );
        ++c;
    }
}

/*-------------------------------------------------------------------------*/

bool
CSceneView::Load( const CORE::CString& groupName, const CORE::CString& sceneName )
{GUCEF_TRACE;

    Unload();
    if ( !m_stage.IsInitialized() )
        return false;
    if ( !Ogre::Codec::isCodecRegistered( "scene" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CSceneView: There is no codec for scenes (is the Ogre plugin OgrePlugin_DotScene loaded?)" );
        return false;
    }

    Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
    const Ogre::String previousWorldGroup = groups.getWorldResourceGroupName();
    try
    {
        Ogre::DataStreamPtr stream = groups.openResource( sceneName.STL_String(), groupName.STL_String() );
        // the scripts of a cooked scene: the materials (and the templates of its particles) must exist before the scene
        CParticlePreview::ParseMaterialsNear( groupName, sceneName );
        CParticlePreview::ParseScriptsNear( groupName, sceneName );
        m_root = m_stage.GetContentNode()->createChildSceneNode();
        // the codec looks the meshes up in the world resource group
        groups.setWorldResourceGroupName( groupName.STL_String() );
        Ogre::Codec::getCodec( "scene" )->decode( stream, Ogre::Any( m_root ) );
        groups.setWorldResourceGroupName( previousWorldGroup );
    }
    catch ( const Ogre::Exception& e )
    {
        groups.setWorldResourceGroupName( previousWorldGroup );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CSceneView: The scene " << sceneName << " cannot be loaded: " << e.getDescription() );
        Unload();
        return false;
    }

    m_groupName = groupName;
    m_sceneName = sceneName;
    m_nodeCount = 0;
    m_objectCount = 0;
    m_meshNames.clear();
    m_root->_update( true, false );
    CSceneTreeFileSystemAccess::TNodeVector nodes;
    AddNodeToTree( m_root, "/", nodes );
    m_tree->SetTree( nodes );
    m_selectedPath = "/";
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CSceneView: Loaded " << sceneName << ": " << m_nodeCount << " nodes, " << m_objectCount << " objects" );
    return true;
}

/*-------------------------------------------------------------------------*/

Ogre::AxisAlignedBox
CSceneView::GetSubtreeBox( Ogre::SceneNode* node )
{GUCEF_TRACE;

    Ogre::AxisAlignedBox box;
    for ( size_t i=0; i<node->numAttachedObjects(); ++i )
    {
        Ogre::MovableObject* object = node->getAttachedObject( i );
        if ( "Light" == object->getMovableType() || "Camera" == object->getMovableType() )
            continue;
        const Ogre::AxisAlignedBox& objectBox = object->getWorldBoundingBox( true );
        if ( objectBox.isFinite() )
            box.merge( objectBox );
    }
    Ogre::Node::ChildNodeMap::const_iterator c = node->getChildren().begin();
    while ( c != node->getChildren().end() )
    {
        const Ogre::AxisAlignedBox childBox = GetSubtreeBox( static_cast< Ogre::SceneNode* >( *c ) );
        if ( childBox.isFinite() )
            box.merge( childBox );
        ++c;
    }
    return box;
}

/*-------------------------------------------------------------------------*/

namespace {

void
CollectObjectBoxes( Ogre::SceneNode* node, GUCEF::vector< Ogre::AxisAlignedBox >& boxes, CORE::CString::StringVector& names )
{
    for ( size_t i=0; i<node->numAttachedObjects(); ++i )
    {
        Ogre::MovableObject* object = node->getAttachedObject( i );
        if ( "Light" == object->getMovableType() || "Camera" == object->getMovableType() )
            continue;
        const Ogre::AxisAlignedBox& objectBox = object->getWorldBoundingBox( true );
        if ( !objectBox.isFinite() || objectBox.isNull() )
            continue;
        boxes.push_back( objectBox );
        names.push_back( CORE::CString( object->getName() ) + "/" + CORE::CString( node->getName() ) );
    }
    Ogre::Node::ChildNodeMap::const_iterator c = node->getChildren().begin();
    while ( c != node->getChildren().end() )
    {
        CollectObjectBoxes( static_cast< Ogre::SceneNode* >( *c ), boxes, names );
        ++c;
    }
}

Float32
BiggestSide( const Ogre::AxisAlignedBox& box )
{
    const Ogre::Vector3 size = box.getSize();
    return std::max( size.x, std::max( size.y, size.z ) );
}

}

/*-------------------------------------------------------------------------*/

bool
CSceneView::GetFramingBox( const GUCEF::vector< Ogre::AxisAlignedBox >& boxes, const CORE::CString::StringVector& names, Float32 maxRatio, Ogre::AxisAlignedBox& box, UInt32& left )
{GUCEF_TRACE;

    box = Ogre::AxisAlignedBox();
    left = 0;
    if ( boxes.empty() )
        return false;
    GUCEF::vector< Float32 > sides;
    for ( size_t i=0; i<boxes.size(); ++i )
        sides.push_back( BiggestSide( boxes[ i ] ) );
    GUCEF::vector< Float32 > sorted = sides;
    std::sort( sorted.begin(), sorted.end() );
    const Float32 median = sorted[ sorted.size() / 2 ];
    const Float32 limit = median > 1e-6f ? median * maxRatio : 1e30f;

    Ogre::AxisAlignedBox kept;
    for ( size_t i=0; i<boxes.size(); ++i )
    {
        const CORE::CString lower = i < names.size() ? names[ i ].Lowercase() : CORE::CString::Empty;
        const bool sky = lower.HasSubstr( "sky" ) >= 0 || lower.HasSubstr( "cloud" ) >= 0;
        if ( sky || sides[ i ] > limit )
        {
            ++left;
            continue;
        }
        kept.merge( boxes[ i ] );
    }
    if ( kept.isNull() )
    {
        // everything is a sky or giant: the scene is that
        left = 0;
        for ( size_t i=0; i<boxes.size(); ++i )
            kept.merge( boxes[ i ] );
    }
    box = kept;
    return !box.isNull();
}

/*-------------------------------------------------------------------------*/

Float32
CSceneView::GetReach( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
        return 0.0f;
    COrbitController::SVector3 center;
    Float32 radius = 1.0f;
    GetBoundingSphere( center, radius );
    const Ogre::AxisAlignedBox all = GetSubtreeBox( m_root );
    if ( !all.isFinite() || all.isNull() )
        return 0.0f;
    Float32 reach = 0.0f;
    for ( UInt32 c=0; c<8; ++c )
    {
        const Ogre::Vector3 corner = all.getAllCorners()[ c ];
        const Float32 dx = corner.x - center.x, dy = corner.y - center.y, dz = corner.z - center.z;
        reach = std::max( reach, sqrtf( dx*dx + dy*dy + dz*dz ) );
    }
    return reach;
}

/*-------------------------------------------------------------------------*/

bool
CSceneView::GetBoundingBox( COrbitController::SVector3& minimum, COrbitController::SVector3& maximum ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
        return false;
    GUCEF::vector< Ogre::AxisAlignedBox > boxes;
    CORE::CString::StringVector names;
    CollectObjectBoxes( m_root, boxes, names );
    Ogre::AxisAlignedBox box;
    UInt32 left = 0;
    if ( !GetFramingBox( boxes, names, 20.0f, box, left ) || !box.isFinite() )
        return false;
    minimum = COrbitController::SVector3( box.getMinimum().x, box.getMinimum().y, box.getMinimum().z );
    maximum = COrbitController::SVector3( box.getMaximum().x, box.getMaximum().y, box.getMaximum().z );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSceneView::GetBoundingSphere( COrbitController::SVector3& center, Float32& radius ) const
{GUCEF_TRACE;

    center = COrbitController::SVector3();
    radius = 1.0f;
    COrbitController::SVector3 low, high;
    if ( !GetBoundingBox( low, high ) )
        return;
    center = COrbitController::SVector3( 0.5f * ( low.x + high.x ), 0.5f * ( low.y + high.y ), 0.5f * ( low.z + high.z ) );
    const Float32 dx = high.x - low.x, dy = high.y - low.y, dz = high.z - low.z;
    radius = 0.5f * sqrtf( dx * dx + dy * dy + dz * dz );
    if ( radius < 1e-6f )
        radius = 1.0f;
}

/*-------------------------------------------------------------------------*/

Ogre::SceneNode*
CSceneView::FindNode( const CORE::CString& path ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
        return GUCEF_NULL;
    if ( "/" == path )
        return m_root;
    const CORE::CString::StringVector parts = path.ParseElements( '/', false );
    Ogre::SceneNode* node = m_root;
    for ( size_t i=0; i<parts.size(); ++i )
    {
        Ogre::SceneNode* next = GUCEF_NULL;
        Ogre::Node::ChildNodeMap::const_iterator c = node->getChildren().begin();
        while ( c != node->getChildren().end() )
        {
            if ( (*c)->getName() == parts[ i ].STL_String() )
                next = static_cast< Ogre::SceneNode* >( *c );
            ++c;
        }
        if ( GUCEF_NULL == next )
            return GUCEF_NULL;
        node = next;
    }
    return node;
}

/*-------------------------------------------------------------------------*/

Ogre::MovableObject*
CSceneView::FindObject( const CORE::CString& filePath ) const
{GUCEF_TRACE;

    const CORE::CString nodePath = CSceneTreeFileSystemAccess::ParentOf( filePath );
    Ogre::SceneNode* node = FindNode( nodePath );
    if ( GUCEF_NULL == node )
        return GUCEF_NULL;
    const CORE::CString fileName = filePath.SubstrFromRange( filePath.HasChar( '/', false ) + 1, filePath.Length() );
    for ( size_t i=0; i<node->numAttachedObjects(); ++i )
    {
        Ogre::MovableObject* object = node->getAttachedObject( i );
        if ( fileName == CORE::CString( object->getName() ) + "." + GetObjectKind( *object ) )
            return object;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CSceneView::ShowBox( const Ogre::AxisAlignedBox& box )
{GUCEF_TRACE;

    Ogre::ManualObject* object = m_stage.GetLineObject( SelectionLines );
    if ( GUCEF_NULL == object )
        return;
    CWorkbenchStage::TLineVertexVector lines;
    CWorkbenchStage::AddBoxLines( lines, box, 1.0f, 0.55f, 0.1f );
    CWorkbenchStage::FillLineObject( *object, lines );
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

bool
CSceneView::Select( const CORE::CString& path )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
        return false;
    Ogre::SceneNode* node = FindNode( path );
    if ( GUCEF_NULL != node )
    {
        m_selectedPath = path;
        ShowBox( GetSubtreeBox( node ) );
        return true;
    }
    Ogre::MovableObject* object = FindObject( path );
    if ( GUCEF_NULL != object )
    {
        m_selectedPath = path;
        ShowBox( object->getWorldBoundingBox( true ) );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSceneView::GetSelectedPath( void ) const
{GUCEF_TRACE;

    return m_selectedPath;
}

/*-------------------------------------------------------------------------*/

void
CSceneView::WriteInspectorData( CORE::CDataNode& root ) const
{GUCEF_TRACE;

    root.SetName( "Scene" );
    root.SetAttribute( "Name", m_sceneName );
    CORE::CDataNode* scene = root.AddChild( "Scene" );
    scene->SetAttribute( "Nodes", CORE::UInt32ToString( m_nodeCount ) );
    scene->SetAttribute( "Objects", CORE::UInt32ToString( m_objectCount ) );
    scene->SetAttribute( "Selected", m_selectedPath );
    if ( GUCEF_NULL == m_root )
        return;

    Ogre::SceneNode* node = FindNode( m_selectedPath );
    if ( GUCEF_NULL != node )
    {
        CORE::CDataNode* properties = root.AddChild( "Node" );
        properties->SetAttribute( "Name", "/" == m_selectedPath ? CORE::CString( "(scene)" ) : CORE::CString( node->getName() ) );
        properties->SetAttribute( "Position", VectorText( node->getPosition() ) );
        const Ogre::Quaternion& q = node->getOrientation();
        properties->SetAttribute( "Orientation", Number( q.w ) + ", " + Number( q.x ) + ", " + Number( q.y ) + ", " + Number( q.z ) );
        properties->SetAttribute( "Scale", VectorText( node->getScale() ) );
        properties->SetAttribute( "WorldPosition", VectorText( node->_getDerivedPosition() ) );
        properties->SetAttribute( "Children", CORE::UInt32ToString( (UInt32) node->getChildren().size() ) );
        CORE::CDataNode* attached = root.AddChild( "Attached" );
        for ( size_t i=0; i<node->numAttachedObjects(); ++i )
            attached->SetAttribute( node->getAttachedObject( i )->getName(), GetObjectKind( *node->getAttachedObject( i ) ) );
        return;
    }

    Ogre::MovableObject* object = FindObject( m_selectedPath );
    if ( GUCEF_NULL == object )
        return;
    CORE::CDataNode* properties = root.AddChild( "Object" );
    properties->SetAttribute( "Name", CORE::CString( object->getName() ) );
    properties->SetAttribute( "Kind", GetObjectKind( *object ) );
    properties->SetAttribute( "Visible", object->getVisible() ? "true" : "false" );
    const Ogre::AxisAlignedBox& box = object->getWorldBoundingBox( true );
    if ( box.isFinite() && !box.isNull() )
        properties->SetAttribute( "Size", VectorText( box.getSize() ) );
    if ( "Entity" == object->getMovableType() )
    {
        Ogre::Entity* entity = static_cast< Ogre::Entity* >( object );
        properties->SetAttribute( "Mesh", CORE::CString( entity->getMesh()->getName() ) );
        properties->SetAttribute( "Submeshes", CORE::UInt32ToString( (UInt32) entity->getNumSubEntities() ) );
    }
    else
    if ( "Light" == object->getMovableType() )
    {
        Ogre::Light* light = static_cast< Ogre::Light* >( object );
        const char* const types[] = { "point", "directional", "spot" };
        properties->SetAttribute( "LightType", types[ (UInt32) light->getType() < 3 ? (UInt32) light->getType() : 0 ] );
        const Ogre::ColourValue& diffuse = light->getDiffuseColour();
        properties->SetAttribute( "Diffuse", Number( diffuse.r ) + ", " + Number( diffuse.g ) + ", " + Number( diffuse.b ) );
        properties->SetAttribute( "Range", Number( light->getAttenuationRange() ) );
    }
    else
    if ( "ParticleSystem" == object->getMovableType() )
    {
        Ogre::ParticleSystem* system = static_cast< Ogre::ParticleSystem* >( object );
        properties->SetAttribute( "Particles", CORE::UInt32ToString( (UInt32) system->getNumParticles() ) );
        properties->SetAttribute( "Quota", CORE::UInt32ToString( (UInt32) system->getParticleQuota() ) );
        properties->SetAttribute( "Emitters", CORE::UInt32ToString( (UInt32) system->getNumEmitters() ) );
        properties->SetAttribute( "Affectors", CORE::UInt32ToString( (UInt32) system->getNumAffectors() ) );
        properties->SetAttribute( "Material", CORE::CString( system->getMaterialName() ) );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
