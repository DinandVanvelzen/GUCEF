/*
 *  AssetViewerLib: the model view: an Ogre mesh in a stage with display modes, bounds, bones, animation, LOD and sub mesh controls
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

#include <Ogre.h>

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

#ifndef ASSETVIEWERLIB_CVIEWERMATERIALS_H
#include "AssetViewerLib_CViewerMaterials.h"
#define ASSETVIEWERLIB_CVIEWERMATERIALS_H
#endif /* ASSETVIEWERLIB_CVIEWERMATERIALS_H ? */

#include "AssetViewerLib_CModelView.h"

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

const UInt32 MaxVectorLines = 60000;
const UInt32 MaxMaterialChoices = 300;

CORE::CString
Number( Float64 value, Int32 decimals )
{
    return GUI::CPropertyRowDescriptor::NumberToText( value, decimals );
}

/**
 *  A nice size for the cells of the grid: 1, 2 or 5 times a power of ten, near an eighth of the width of the model. The grid then
 *  reaches about as far as the model is wide: all of it is in front of the camera of the framed view, where the lines do not
 *  have to be cut by the near plane (software rasterizers of Vulkan draw those with artifacts)
 */
Float32
NiceGridUnit( Float32 size )
{
    if ( size <= 0.0f )
        return 1.0f;
    const Float32 target = size / 8.0f;
    const Float32 magnitude = powf( 10.0f, floorf( log10f( target ) ) );
    const Float32 normalized = target / magnitude;
    const Float32 nice = normalized < 1.5f ? 1.0f : ( normalized < 3.5f ? 2.0f : ( normalized < 7.5f ? 5.0f : 10.0f ) );
    return nice * magnitude;
}

void
DrawLine( Ogre::uchar* pixels, UInt32 size, Float32 x0, Float32 y0, Float32 x1, Float32 y1, UInt8 r, UInt8 g, UInt8 b )
{
    // a line in steps of half a pixel: the layout is a picture to look at, not a rasterizer to be exact
    const Float32 dx = x1 - x0;
    const Float32 dy = y1 - y0;
    const Float32 length = sqrtf( dx*dx + dy*dy );
    const UInt32 steps = (UInt32) ( length * 2.0f ) + 1;
    for ( UInt32 i=0; i<=steps; ++i )
    {
        const Float32 t = (Float32) i / (Float32) steps;
        const Int32 x = (Int32) ( x0 + dx * t + 0.5f );
        const Int32 y = (Int32) ( y0 + dy * t + 0.5f );
        if ( x < 0 || y < 0 || x >= (Int32) size || y >= (Int32) size )
            continue;
        Ogre::uchar* pixel = pixels + ( (size_t) y * size + (size_t) x ) * 4;
        pixel[ 0 ] = r;
        pixel[ 1 ] = g;
        pixel[ 2 ] = b;
        pixel[ 3 ] = 255;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CModelView::CModelView( CViewerStage& stage )
    : m_stage( stage )
    , m_mesh()
    , m_entity( GUCEF_NULL )
    , m_node( GUCEF_NULL )
    , m_meshName()
    , m_groupName()
    , m_statistics()
    , m_displayMode( DISPLAYMODE_SHADED )
    , m_boundsVisible( false )
    , m_skeletonVisible( false )
    , m_forcedLod( -1 )
    , m_cameraDistance( 0.0f )
    , m_selectedClip( -1 )
    , m_clips()
    , m_baseMaterials()
    , m_subMeshVisible()
    , m_ownedMaterials()
    , m_meshIsManual( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CModelView::~CModelView()
{GUCEF_TRACE;

    Unload();
}

/*-------------------------------------------------------------------------*/

bool
CModelView::LoadMesh( const CORE::CString& groupName, const CORE::CString& meshName )
{GUCEF_TRACE;

    Unload();
    if ( !m_stage.IsInitialized() )
        return false;

    try
    {
        // what the loader makes besides the mesh (the materials of a source model) goes away with it
        GUCEF::set< Ogre::String > materialsBefore;
        {
            Ogre::ResourceManager::ResourceMapIterator materials = Ogre::MaterialManager::getSingleton().getResourceIterator();
            while ( materials.hasMoreElements() )
                materialsBefore.insert( materials.getNext()->getName() );
        }

        // a mesh that the application made itself (a prefab, a converted manual object) is shown as it is; one that was read from a file
        // is read again, as the file may have changed
        Ogre::MeshManager& meshes = Ogre::MeshManager::getSingleton();
        Ogre::MeshPtr existing = meshes.getByName( meshName.STL_String(), groupName.STL_String() );
        m_meshIsManual = existing && existing->isManuallyLoaded();
        if ( m_meshIsManual )
            m_mesh = existing;
        else
        {
            if ( existing )
                meshes.remove( existing );
            m_mesh = meshes.load( meshName.STL_String(), groupName.STL_String() );
        }
        if ( !m_mesh )
            return false;

        Ogre::ResourceManager::ResourceMapIterator materialsAfter = Ogre::MaterialManager::getSingleton().getResourceIterator();
        while ( materialsAfter.hasMoreElements() )
        {
            const Ogre::String name = materialsAfter.getNext()->getName();
            if ( materialsBefore.find( name ) == materialsBefore.end() )
                m_ownedMaterials.push_back( name );
        }

        m_meshName = meshName;
        m_groupName = groupName;
        m_entity = m_stage.GetSceneManager()->createEntity( m_mesh );
        m_node = m_stage.GetContentNode()->createChildSceneNode();
        m_node->attachObject( m_entity );

        const UInt32 subMeshCount = (UInt32) m_entity->getNumSubEntities();
        m_baseMaterials.clear();
        m_baseMaterialGroups.clear();
        m_subMeshVisible.assign( subMeshCount, true );
        for ( UInt32 i=0; i<subMeshCount; ++i )
        {
            m_baseMaterials.push_back( m_entity->getSubEntity( i )->getMaterialName() );
            m_baseMaterialGroups.push_back( m_entity->getSubEntity( i )->getMaterial() ? m_entity->getSubEntity( i )->getMaterial()->getGroup() : Ogre::String() );
        }

        RemoveMissingTextures();
        CMeshInspector::GetStatistics( m_mesh, m_statistics );
        CollectClips();
        m_forcedLod = -1;
        m_selectedClip = -1;
        m_displayMode = DISPLAYMODE_SHADED;
        m_stage.SetWireframe( false );
        ApplyMaterials();
        m_stage.MarkDirty();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CModelView: Loaded " << meshName << ": " << m_statistics.vertexCount << " vertices, " << m_statistics.triangleCount
            << " triangles, " << m_statistics.subMeshCount << " sub meshes, " << m_statistics.boneCount << " bones, " << m_statistics.materialCount << " materials, "
            << m_statistics.textureCount << " textures" );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CModelView: The model " << meshName << " cannot be loaded: " << e.getDescription() );
        Unload();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CModelView::RemoveMissingTextures( void )
{GUCEF_TRACE;

    // a source model names the pictures its author had on the disk: what is not in the mount is taken off the material, which then
    // shows its own colours, instead of a layer that Ogre fills with its blank texture
    Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
    GUCEF::set< Ogre::String > warned;
    for ( size_t i=0; i<m_baseMaterials.size(); ++i )
    {
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName( m_baseMaterials[ i ].STL_String(), m_baseMaterialGroups[ i ].STL_String() );
        if ( !material )
            continue;
        for ( unsigned short t=0; t<material->getNumTechniques(); ++t )
        {
            Ogre::Technique* technique = material->getTechnique( t );
            for ( unsigned short p=0; p<technique->getNumPasses(); ++p )
            {
                Ogre::Pass* pass = technique->getPass( p );
                for ( int u=(int) pass->getNumTextureUnitStates()-1; u>=0; --u )
                {
                    Ogre::TextureUnitState* unit = pass->getTextureUnitState( (unsigned short) u );
                    if ( 0 == unit->getNumFrames() || unit->getTextureName().empty() )
                        continue;
                    const Ogre::String name = unit->getTextureName();
                    // a file of the mount, or a picture that came inside of the model and was made by the loader
                    Ogre::TexturePtr known = Ogre::TextureManager::getSingleton().getByName( name, m_groupName.STL_String() );
                    if ( groups.resourceExists( m_groupName.STL_String(), name ) || ( known && known->isLoaded() ) )
                        continue;
                    if ( warned.insert( name ).second )
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CModelView: The picture " << name << " of material " << m_baseMaterials[ i ] << " is not in the mount: the material is shown without it" );
                    pass->removeTextureUnitState( (unsigned short) u );
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CModelView::ClearLines( void )
{GUCEF_TRACE;

    m_stage.ClearLineObjects();
}

/*-------------------------------------------------------------------------*/

void
CModelView::Unload( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != Ogre::Root::getSingletonPtr() && m_stage.IsInitialized() )
    {
        try
        {
            ClearLines();
            if ( GUCEF_NULL != m_entity )
            {
                m_node->detachObject( m_entity );
                m_stage.GetSceneManager()->destroyEntity( m_entity );
                m_stage.GetContentNode()->removeAndDestroyChild( m_node );
            }
            m_entity = GUCEF_NULL;
            m_node = GUCEF_NULL;

            // the mesh, its skeleton and the materials its loader made, and the textures they used are let go of the GPU
            if ( m_mesh && !m_meshIsManual )
            {
                const Ogre::String skeletonName = m_mesh->getSkeletonName();
                Ogre::MeshManager::getSingleton().remove( m_mesh );
                if ( !skeletonName.empty() && Ogre::SkeletonManager::getSingleton().resourceExists( skeletonName, m_groupName.STL_String() ) )
                    Ogre::SkeletonManager::getSingleton().remove( skeletonName, m_groupName.STL_String() );
            }
            for ( size_t i=0; i<m_ownedMaterials.size(); ++i )
                if ( Ogre::MaterialManager::getSingleton().resourceExists( m_ownedMaterials[ i ].STL_String() ) )
                    Ogre::MaterialManager::getSingleton().remove( m_ownedMaterials[ i ].STL_String() );
            for ( size_t i=0; i<m_statistics.textureNames.size(); ++i )
            {
                Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName( m_statistics.textureNames[ i ].STL_String(), m_groupName.STL_String() );
                if ( texture )
                    texture->unload();
            }
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CModelView: The model cannot be taken out of the stage: " << e.getDescription() );
        }
    }
    m_entity = GUCEF_NULL;
    m_node = GUCEF_NULL;
    m_mesh.reset();
    m_meshName.Clear();
    m_statistics = SMeshStatistics();
    m_clips.clear();
    m_baseMaterials.clear();
    m_baseMaterialGroups.clear();
    m_subMeshVisible.clear();
    m_ownedMaterials.clear();
    m_selectedClip = -1;
    m_forcedLod = -1;
}

/*-------------------------------------------------------------------------*/

bool
CModelView::IsLoaded( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_entity;
}

/*-------------------------------------------------------------------------*/

const Ogre::MeshPtr&
CModelView::GetMesh( void ) const
{GUCEF_TRACE;

    return m_mesh;
}

/*-------------------------------------------------------------------------*/

Ogre::Entity*
CModelView::GetEntity( void ) const
{GUCEF_TRACE;

    return m_entity;
}

/*-------------------------------------------------------------------------*/

const SMeshStatistics&
CModelView::GetStatistics( void ) const
{GUCEF_TRACE;

    return m_statistics;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CModelView::GetMeshName( void ) const
{GUCEF_TRACE;

    return m_meshName;
}

/*-------------------------------------------------------------------------*/

void
CModelView::GetBoundingSphere( COrbitController::SVector3& center, Float32& radius ) const
{GUCEF_TRACE;

    center = COrbitController::SVector3();
    radius = 1.0f;
    if ( !m_mesh )
        return;
    const Ogre::AxisAlignedBox& bounds = m_mesh->getBounds();
    if ( !bounds.isFinite() || bounds.isNull() )
        return;
    const Ogre::Vector3 middle = bounds.getCenter();
    center = COrbitController::SVector3( middle.x, middle.y, middle.z );
    radius = 0.5f * bounds.getSize().length();
    if ( radius < 1e-6f )
        radius = 1.0f;
}

/*-------------------------------------------------------------------------*/

void
CModelView::FrameInController( COrbitController& controller )
{GUCEF_TRACE;

    COrbitController::SVector3 center;
    Float32 radius = 1.0f;
    GetBoundingSphere( center, radius );
    controller.SetBounds( center, radius );
    m_stage.SetGridUnit( NiceGridUnit( 2.0f * radius ) );
}

/*-------------------------------------------------------------------------*/

void
CModelView::ApplyMaterials( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_entity )
        return;
    CViewerMaterials::Ensure();
    for ( UInt32 i=0; i<(UInt32) m_entity->getNumSubEntities(); ++i )
    {
        CORE::CString material = i < m_baseMaterials.size() ? m_baseMaterials[ i ] : CORE::CString::Empty;
        if ( DISPLAYMODE_UV_CHECKER == m_displayMode )
            material = CViewerMaterials::UvCheckerMaterialName;
        else
        if ( DISPLAYMODE_VERTEX_COLOURS == m_displayMode )
            material = CViewerMaterials::VertexColoursMaterialName;
        try
        {
            // the viewer's own materials are in the internal group; the others in the group their script or loader gave them
            CORE::CString group = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
            if ( DISPLAYMODE_UV_CHECKER == m_displayMode || DISPLAYMODE_VERTEX_COLOURS == m_displayMode )
                group = Ogre::RGN_INTERNAL;
            else
            if ( i < m_baseMaterialGroups.size() && !m_baseMaterialGroups[ i ].IsNULLOrEmpty() )
                group = m_baseMaterialGroups[ i ];
            if ( !material.IsNULLOrEmpty() )
                m_entity->getSubEntity( i )->setMaterialName( material.STL_String(), group.STL_String() );
            m_entity->getSubEntity( i )->setVisible( i < m_subMeshVisible.size() ? m_subMeshVisible[ i ] : true );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CModelView: Sub mesh " << i << " cannot take the material " << material << ": " << e.getDescription() );
        }
    }
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CModelView::SetDisplayMode( EDisplayMode mode )
{GUCEF_TRACE;

    if ( (Int32) mode < 0 || (Int32) mode >= (Int32) DISPLAYMODE_COUNT )
        return;
    m_displayMode = mode;
    m_stage.SetWireframe( DISPLAYMODE_WIREFRAME == mode );
    ApplyMaterials();
    RebuildVectorLines();
}

/*-------------------------------------------------------------------------*/

CModelView::EDisplayMode
CModelView::GetDisplayMode( void ) const
{GUCEF_TRACE;

    return m_displayMode;
}

/*-------------------------------------------------------------------------*/

const char*
CModelView::GetDisplayModeName( EDisplayMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case DISPLAYMODE_SHADED:            return "Shaded";
        case DISPLAYMODE_WIREFRAME:         return "Wireframe";
        case DISPLAYMODE_NORMALS:           return "Normals";
        case DISPLAYMODE_TANGENTS:          return "Tangents";
        case DISPLAYMODE_UV_CHECKER:        return "UV checker";
        case DISPLAYMODE_VERTEX_COLOURS:    return "Vertex colours";
        default:                            return "Shaded";
    }
}

/*-------------------------------------------------------------------------*/

void
CModelView::RebuildVectorLines( void )
{GUCEF_TRACE;

    Ogre::ManualObject* normals = m_stage.GetLineObject( "Normals" );
    Ogre::ManualObject* tangents = m_stage.GetLineObject( "Tangents" );
    if ( GUCEF_NULL == normals || GUCEF_NULL == tangents )
        return;
    CViewerStage::TLineVertexVector lines;
    const bool wantNormals = DISPLAYMODE_NORMALS == m_displayMode;
    const bool wantTangents = DISPLAYMODE_TANGENTS == m_displayMode;
    if ( ( wantNormals || wantTangents ) && m_mesh )
    {
        COrbitController::SVector3 center;
        Float32 radius = 1.0f;
        GetBoundingSphere( center, radius );
        const Float32 length = radius * 0.06f;
        const UInt32 stride = m_statistics.vertexCount > MaxVectorLines ? m_statistics.vertexCount / MaxVectorLines + 1 : 1;
        for ( UInt32 s=0; s<(UInt32) m_subMeshVisible.size(); ++s )
        {
            if ( !m_subMeshVisible[ s ] )
                continue;
            SSubMeshGeometry geometry;
            if ( !CMeshInspector::ReadSubMesh( m_mesh, s, geometry ) )
                continue;
            const GUCEF::vector< Float32 >& vectors = wantNormals ? geometry.normals : geometry.tangents;
            if ( vectors.size() != geometry.positions.size() )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CModelView: Sub mesh " << s << " has no " << ( wantNormals ? "normals" : "tangents" ) << " to show" );
                continue;
            }
            for ( UInt32 v=0; v<geometry.vertexCount; v+=stride )
            {
                const Float32* p = &geometry.positions[ v*3 ];
                const Float32* d = &vectors[ v*3 ];
                CViewerStage::SLineVertex start = { p[ 0 ], p[ 1 ], p[ 2 ], wantNormals ? 0.30f : 0.95f, wantNormals ? 0.90f : 0.35f, wantNormals ? 0.95f : 0.30f };
                CViewerStage::SLineVertex end = { p[ 0 ] + d[ 0 ] * length, p[ 1 ] + d[ 1 ] * length, p[ 2 ] + d[ 2 ] * length, start.r, start.g, start.b };
                lines.push_back( start );
                lines.push_back( end );
            }
        }
    }
    CViewerStage::FillLineObject( wantNormals ? *normals : *tangents, lines );
    CViewerStage::FillLineObject( wantNormals ? *tangents : *normals, CViewerStage::TLineVertexVector() );
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CModelView::RebuildBoundsLines( void )
{GUCEF_TRACE;

    Ogre::ManualObject* object = m_stage.GetLineObject( "Bounds" );
    if ( GUCEF_NULL == object )
        return;
    CViewerStage::TLineVertexVector lines;
    if ( m_boundsVisible && m_mesh )
        CViewerStage::AddBoxLines( lines, m_mesh->getBounds(), 1.0f, 0.85f, 0.2f );
    CViewerStage::FillLineObject( *object, lines );
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CModelView::RebuildSkeletonLines( void )
{GUCEF_TRACE;

    Ogre::ManualObject* object = m_stage.GetLineObject( "Bones" );
    if ( GUCEF_NULL == object )
        return;
    CViewerStage::TLineVertexVector lines;
    if ( m_skeletonVisible && GUCEF_NULL != m_entity && GUCEF_NULL != m_entity->getSkeleton() )
    {
        // the skeleton as the animation has it now
        m_entity->_updateAnimation();
        Ogre::SkeletonInstance* skeleton = m_entity->getSkeleton();
        for ( unsigned short i=0; i<skeleton->getNumBones(); ++i )
        {
            Ogre::Bone* bone = skeleton->getBone( i );
            const Ogre::Node* parent = bone->getParent();
            if ( GUCEF_NULL == parent )
                continue;
            const Ogre::Vector3 from = parent->_getDerivedPosition();
            const Ogre::Vector3 to = bone->_getDerivedPosition();
            CViewerStage::SLineVertex a = { from.x, from.y, from.z, 1.0f, 0.9f, 0.2f };
            CViewerStage::SLineVertex b = { to.x, to.y, to.z, 1.0f, 0.55f, 0.1f };
            lines.push_back( a );
            lines.push_back( b );
        }
    }
    CViewerStage::FillLineObject( *object, lines );
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CModelView::SetBoundsVisible( bool visible )
{GUCEF_TRACE;

    m_boundsVisible = visible;
    RebuildBoundsLines();
}

/*-------------------------------------------------------------------------*/

bool
CModelView::IsBoundsVisible( void ) const
{GUCEF_TRACE;

    return m_boundsVisible;
}

/*-------------------------------------------------------------------------*/

void
CModelView::SetSkeletonVisible( bool visible )
{GUCEF_TRACE;

    m_skeletonVisible = visible;
    RebuildSkeletonLines();
}

/*-------------------------------------------------------------------------*/

bool
CModelView::IsSkeletonVisible( void ) const
{GUCEF_TRACE;

    return m_skeletonVisible;
}

/*-------------------------------------------------------------------------*/

UInt32
CModelView::GetSubMeshCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_subMeshVisible.size();
}

/*-------------------------------------------------------------------------*/

bool
CModelView::IsSubMeshVisible( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_subMeshVisible.size() && m_subMeshVisible[ index ];
}

/*-------------------------------------------------------------------------*/

void
CModelView::SetSubMeshVisible( UInt32 index, bool visible )
{GUCEF_TRACE;

    if ( index >= m_subMeshVisible.size() )
        return;
    m_subMeshVisible[ index ] = visible;
    ApplyMaterials();
    if ( DISPLAYMODE_NORMALS == m_displayMode || DISPLAYMODE_TANGENTS == m_displayMode )
        RebuildVectorLines();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CModelView::GetSubMeshMaterial( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_baseMaterials.size() ? m_baseMaterials[ index ] : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CModelView::SetSubMeshMaterial( UInt32 index, const CORE::CString& materialName, const CORE::CString& groupName )
{GUCEF_TRACE;

    if ( index >= m_baseMaterials.size() || materialName.IsNULLOrEmpty() )
        return false;

    // in the group asked for, in the group of the mesh, or wherever Ogre finds it
    Ogre::MaterialManager& materials = Ogre::MaterialManager::getSingleton();
    CORE::CString found;
    if ( !groupName.IsNULLOrEmpty() && materials.resourceExists( materialName.STL_String(), groupName.STL_String() ) )
        found = groupName;
    else
    if ( !m_groupName.IsNULLOrEmpty() && materials.resourceExists( materialName.STL_String(), m_groupName.STL_String() ) )
        found = m_groupName;
    else
    if ( materials.resourceExists( materialName.STL_String() ) )
        found = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
    else
    if ( !m_baseMaterialGroups[ index ].IsNULLOrEmpty() && materials.resourceExists( materialName.STL_String(), m_baseMaterialGroups[ index ].STL_String() ) )
        found = m_baseMaterialGroups[ index ];
    else
    {
        // the materials of a loader and of scripts are in their own groups: look at the ones the sub meshes have
        for ( size_t i=0; i<m_baseMaterialGroups.size() && found.IsNULLOrEmpty(); ++i )
            if ( !m_baseMaterialGroups[ i ].IsNULLOrEmpty() && materials.resourceExists( materialName.STL_String(), m_baseMaterialGroups[ i ].STL_String() ) )
                found = m_baseMaterialGroups[ i ];
    }
    if ( found.IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CModelView: There is no material " << materialName );
        return false;
    }
    m_baseMaterials[ index ] = materialName;
    m_baseMaterialGroups[ index ] = found;
    ApplyMaterials();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CModelView::GetMaterialChoices( CORE::CString::StringVector& names ) const
{GUCEF_TRACE;

    names.clear();
    GUCEF::set< CORE::CString > sorted;
    for ( size_t i=0; i<m_baseMaterials.size(); ++i )
        sorted.insert( m_baseMaterials[ i ] );
    Ogre::ResourceManager::ResourceMapIterator materials = Ogre::MaterialManager::getSingleton().getResourceIterator();
    while ( materials.hasMoreElements() && sorted.size() < MaxMaterialChoices )
    {
        const Ogre::String name = materials.getNext()->getName();
        if ( 0 != name.compare( 0, 12, "AssetViewer/" ) )
            sorted.insert( name );
    }
    GUCEF::set< CORE::CString >::const_iterator i = sorted.begin();
    while ( i != sorted.end() )
    {
        names.push_back( *i );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CModelView::GetLodLevelCount( void ) const
{GUCEF_TRACE;

    return m_mesh ? (UInt32) m_mesh->getNumLodLevels() : 0;
}

/*-------------------------------------------------------------------------*/

void
CModelView::SetForcedLod( Int32 level )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_entity )
        return;
    if ( level < 0 || level >= (Int32) GetLodLevelCount() )
    {
        m_forcedLod = -1;
        m_entity->setMeshLodBias( 1.0f, 0, 99 );
    }
    else
    {
        m_forcedLod = level;
        m_entity->setMeshLodBias( 1.0f, (unsigned short) level, (unsigned short) level );
    }
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

Int32
CModelView::GetForcedLod( void ) const
{GUCEF_TRACE;

    return m_forcedLod;
}

/*-------------------------------------------------------------------------*/

UInt32
CModelView::GetCurrentLod( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_entity ? (UInt32) m_entity->getCurrentLodIndex() : 0;
}

/*-------------------------------------------------------------------------*/

Float32
CModelView::GetCameraDistance( void ) const
{GUCEF_TRACE;

    return m_cameraDistance;
}

/*-------------------------------------------------------------------------*/

void
CModelView::CollectClips( void )
{GUCEF_TRACE;

    m_clips.clear();
    if ( GUCEF_NULL == m_entity )
        return;
    Ogre::AnimationStateSet* states = m_entity->getAllAnimationStates();
    if ( GUCEF_NULL == states )
        return;
    const Ogre::AnimationStateMap& map = states->getAnimationStates();
    Ogre::AnimationStateMap::const_iterator i = map.begin();
    while ( i != map.end() )
    {
        SClip clip;
        clip.name = i->first;
        clip.lengthInMs = (UInt32) ( i->second->getLength() * 1000.0f + 0.5f );
        m_clips.push_back( clip );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CModelView::TClipVector&
CModelView::GetClips( void ) const
{GUCEF_TRACE;

    return m_clips;
}

/*-------------------------------------------------------------------------*/

bool
CModelView::SelectClip( Int32 index )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_entity )
        return false;
    Ogre::AnimationStateSet* states = m_entity->getAllAnimationStates();
    if ( GUCEF_NULL == states )
        return false;
    if ( index >= (Int32) m_clips.size() )
        return false;
    // one clip at a time: the others are switched off
    if ( m_selectedClip >= 0 && m_selectedClip < (Int32) m_clips.size() && states->hasAnimationState( m_clips[ m_selectedClip ].name.STL_String() ) )
        states->getAnimationState( m_clips[ m_selectedClip ].name.STL_String() )->setEnabled( false );
    m_selectedClip = index;
    if ( index >= 0 )
    {
        Ogre::AnimationState* state = states->getAnimationState( m_clips[ index ].name.STL_String() );
        state->setEnabled( true );
        state->setLoop( true );
        state->setTimePosition( 0.0f );
    }
    RebuildSkeletonLines();
    m_stage.MarkDirty();
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CModelView::GetSelectedClip( void ) const
{GUCEF_TRACE;

    return m_selectedClip;
}

/*-------------------------------------------------------------------------*/

void
CModelView::SetClipTime( UInt32 timeInMs )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_entity || m_selectedClip < 0 || m_selectedClip >= (Int32) m_clips.size() )
        return;
    Ogre::AnimationStateSet* states = m_entity->getAllAnimationStates();
    if ( GUCEF_NULL == states )
        return;
    states->getAnimationState( m_clips[ m_selectedClip ].name.STL_String() )->setTimePosition( (Float32) timeInMs / 1000.0f );
    if ( m_skeletonVisible )
        RebuildSkeletonLines();
    m_stage.MarkDirty();
}

/*-------------------------------------------------------------------------*/

void
CModelView::Update( const COrbitController& controller )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_entity )
        return;
    const COrbitController::SVector3 eye = controller.GetEye();
    const COrbitController::SVector3& center = controller.GetBoundsCenter();
    const Float32 dx = eye.x - center.x;
    const Float32 dy = eye.y - center.y;
    const Float32 dz = eye.z - center.z;
    m_cameraDistance = sqrtf( dx*dx + dy*dy + dz*dz );
}

/*-------------------------------------------------------------------------*/

void
CModelView::WriteInspectorData( CORE::CDataNode& root ) const
{GUCEF_TRACE;

    root.SetName( "Model" );
    root.SetAttribute( "Name", m_meshName );

    CORE::CDataNode* view = root.AddChild( "View" );
    view->SetAttribute( "Lod", m_forcedLod < 0 ? CORE::CString( "Auto" ) : CORE::Int32ToString( m_forcedLod ) );
    view->SetAttribute( "CurrentLod", CORE::UInt32ToString( GetCurrentLod() ) );
    view->SetAttribute( "CameraDistance", Number( m_cameraDistance, 3 ) );

    CMeshInspector::WriteToDataNode( m_statistics, root );

    CORE::CDataNode* submeshes = root.AddChild( "Submeshes" );
    for ( UInt32 i=0; i<GetSubMeshCount(); ++i )
    {
        CORE::CDataNode* entry = submeshes->AddChild( "Submesh" + CORE::UInt32ToString( i ) );
        if ( m_mesh )
        {
            Ogre::SubMesh* subMesh = m_mesh->getSubMesh( (unsigned short) i );
            const Ogre::VertexData* vertices = CMeshInspector::GetVertexData( m_mesh, i );
            entry->SetAttribute( "Vertices", CORE::UInt32ToString( GUCEF_NULL != vertices ? (UInt32) vertices->vertexCount : 0 ) );
            entry->SetAttribute( "Triangles", CORE::UInt32ToString( CMeshInspector::CountTriangles( subMesh->operationType, (UInt32) subMesh->indexData->indexCount ) ) );
        }
        entry->SetAttribute( "Visible", IsSubMeshVisible( i ) ? "true" : "false" );
        entry->SetAttribute( "Material", GetSubMeshMaterial( i ) );
    }

    if ( !m_clips.empty() )
    {
        CORE::CDataNode* clips = root.AddChild( "Clips" );
        for ( size_t i=0; i<m_clips.size(); ++i )
            clips->SetAttribute( m_clips[ i ].name, Number( (Float64) m_clips[ i ].lengthInMs / 1000.0, 2 ) + " s" );
    }
}

/*-------------------------------------------------------------------------*/

void
CModelView::GetInspectorDescriptors( GUI::CPropertyInspectorForm::TDescriptorMap& descriptors ) const
{GUCEF_TRACE;

    descriptors.clear();
    CORE::CString::StringVector lods;
    lods.push_back( "Auto" );
    for ( UInt32 i=0; i<GetLodLevelCount(); ++i )
        lods.push_back( CORE::UInt32ToString( i ) );
    descriptors[ "View@Lod" ] = GUI::CPropertyRowDescriptor::Choice( lods ).WithTooltip( "Automatic by the distance of the camera, or one level forced" );
    descriptors[ "View@CurrentLod" ] = GUI::CPropertyRowDescriptor::Text().AsReadOnly().WithTooltip( "The level drawn at the last frame" );
    descriptors[ "View@CameraDistance" ] = GUI::CPropertyRowDescriptor::Text().AsReadOnly().WithTooltip( "Distance of the camera to the middle of the model" );

    CORE::CString::StringVector materials;
    GetMaterialChoices( materials );
    for ( UInt32 i=0; i<GetSubMeshCount(); ++i )
    {
        const CORE::CString path = "Submeshes/Submesh" + CORE::UInt32ToString( i );
        descriptors[ path + "@Visible" ] = GUI::CPropertyRowDescriptor::Boolean();
        descriptors[ path + "@Material" ] = GUI::CPropertyRowDescriptor::Choice( materials );
    }
}

/*-------------------------------------------------------------------------*/

bool
CModelView::ApplyProperty( const CORE::CString& path, const CORE::CString& value )
{GUCEF_TRACE;

    if ( "View@Lod" == path )
    {
        SetForcedLod( "Auto" == value ? -1 : CORE::StringToInt32( value, -1 ) );
        return true;
    }
    const CORE::CString prefix = "Submeshes/Submesh";
    if ( path.Length() > prefix.Length() && path.SubstrFromRange( 0, prefix.Length() ) == prefix )
    {
        const Int32 at = path.HasChar( '@', true );
        if ( at <= (Int32) prefix.Length() )
            return false;
        const UInt32 index = CORE::StringToUInt32( path.SubstrFromRange( prefix.Length(), (UInt32) at ), 0xFFFFFFFFU );
        const CORE::CString attribute = path.SubstrFromRange( (UInt32) at+1, path.Length() );
        if ( "Visible" == attribute )
        {
            SetSubMeshVisible( index, CORE::StringToBool( value, true ) );
            return true;
        }
        if ( "Material" == attribute )
            return SetSubMeshMaterial( index, value );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CModelView::RenderUvLayout( UInt32 subMeshIndex, UInt32 size, IMAGE::TPixelMapPtr& pixels ) const
{GUCEF_TRACE;

    SSubMeshGeometry geometry;
    if ( size < 8 || !CMeshInspector::ReadSubMesh( m_mesh, subMeshIndex, geometry ) || geometry.texCoords.size() < (size_t) geometry.vertexCount * 2 )
        return false;

    pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, size, size, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( size_t i=0; i<(size_t) size*size; ++i )
    {
        data[ i*4 ] = 24;
        data[ i*4+1 ] = 26;
        data[ i*4+2 ] = 32;
        data[ i*4+3 ] = 255;
    }

    // the unit square of the texture, then the edges of every triangle in the texture space (v down, as Ogre has it)
    const Float32 last = (Float32) ( size - 1 );
    DrawLine( data, size, 0.0f, 0.0f, last, 0.0f, 90, 94, 110 );
    DrawLine( data, size, last, 0.0f, last, last, 90, 94, 110 );
    DrawLine( data, size, last, last, 0.0f, last, 90, 94, 110 );
    DrawLine( data, size, 0.0f, last, 0.0f, 0.0f, 90, 94, 110 );
    for ( size_t t=0; t+2<geometry.indices.size(); t+=3 )
    {
        Float32 u[ 3 ], v[ 3 ];
        for ( UInt32 c=0; c<3; ++c )
        {
            const UInt32 index = geometry.indices[ t+c ];
            u[ c ] = geometry.texCoords[ (size_t) index*2 ] * last;
            v[ c ] = geometry.texCoords[ (size_t) index*2+1 ] * last;
        }
        for ( UInt32 c=0; c<3; ++c )
            DrawLine( data, size, u[ c ], v[ c ], u[ ( c+1 ) % 3 ], v[ ( c+1 ) % 3 ], 120, 200, 255 );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
