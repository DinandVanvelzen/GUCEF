/*
 *  AssetWorkbenchLib: the models of a generated showcase in one stage
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

#ifndef ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#include "AssetWorkbenchLib_CAssetTypeResolver.h"
#define ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#endif /* ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHMATERIALS_H
#include "AssetWorkbenchLib_CWorkbenchMaterials.h"
#define ASSETWORKBENCHLIB_CWORKBENCHMATERIALS_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHMATERIALS_H ? */

#ifndef ASSETWORKBENCHLIB_CMESHINSPECTOR_H
#include "AssetWorkbenchLib_CMeshInspector.h"
#define ASSETWORKBENCHLIB_CMESHINSPECTOR_H
#endif /* ASSETWORKBENCHLIB_CMESHINSPECTOR_H ? */

#include "AssetWorkbenchLib_CShowcaseView.h"

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

const char* const LabelLineObjectName = "showcase_labels";

/** Capitals, digits and a few marks as five dots wide and seven high: one string of five characters per row, 'X' is a dot */
struct SGlyph
{
    char character;
    const char* rows[ 7 ];
};

const SGlyph Glyphs[] = {
    { 'A', { ".XXX.", "X...X", "X...X", "XXXXX", "X...X", "X...X", "X...X" } },
    { 'B', { "XXXX.", "X...X", "X...X", "XXXX.", "X...X", "X...X", "XXXX." } },
    { 'C', { ".XXX.", "X...X", "X....", "X....", "X....", "X...X", ".XXX." } },
    { 'D', { "XXXX.", "X...X", "X...X", "X...X", "X...X", "X...X", "XXXX." } },
    { 'E', { "XXXXX", "X....", "X....", "XXXX.", "X....", "X....", "XXXXX" } },
    { 'F', { "XXXXX", "X....", "X....", "XXXX.", "X....", "X....", "X...." } },
    { 'G', { ".XXX.", "X...X", "X....", "X.XXX", "X...X", "X...X", ".XXXX" } },
    { 'H', { "X...X", "X...X", "X...X", "XXXXX", "X...X", "X...X", "X...X" } },
    { 'I', { ".XXX.", "..X..", "..X..", "..X..", "..X..", "..X..", ".XXX." } },
    { 'J', { "..XXX", "...X.", "...X.", "...X.", "...X.", "X..X.", ".XX.." } },
    { 'K', { "X...X", "X..X.", "X.X..", "XX...", "X.X..", "X..X.", "X...X" } },
    { 'L', { "X....", "X....", "X....", "X....", "X....", "X....", "XXXXX" } },
    { 'M', { "X...X", "XX.XX", "X.X.X", "X.X.X", "X...X", "X...X", "X...X" } },
    { 'N', { "X...X", "XX..X", "X.X.X", "X..XX", "X...X", "X...X", "X...X" } },
    { 'O', { ".XXX.", "X...X", "X...X", "X...X", "X...X", "X...X", ".XXX." } },
    { 'P', { "XXXX.", "X...X", "X...X", "XXXX.", "X....", "X....", "X...." } },
    { 'Q', { ".XXX.", "X...X", "X...X", "X...X", "X.X.X", "X..X.", ".XX.X" } },
    { 'R', { "XXXX.", "X...X", "X...X", "XXXX.", "X.X..", "X..X.", "X...X" } },
    { 'S', { ".XXXX", "X....", "X....", ".XXX.", "....X", "....X", "XXXX." } },
    { 'T', { "XXXXX", "..X..", "..X..", "..X..", "..X..", "..X..", "..X.." } },
    { 'U', { "X...X", "X...X", "X...X", "X...X", "X...X", "X...X", ".XXX." } },
    { 'V', { "X...X", "X...X", "X...X", "X...X", "X...X", ".X.X.", "..X.." } },
    { 'W', { "X...X", "X...X", "X...X", "X.X.X", "X.X.X", "XX.XX", "X...X" } },
    { 'X', { "X...X", "X...X", ".X.X.", "..X..", ".X.X.", "X...X", "X...X" } },
    { 'Y', { "X...X", "X...X", ".X.X.", "..X..", "..X..", "..X..", "..X.." } },
    { 'Z', { "XXXXX", "....X", "...X.", "..X..", ".X...", "X....", "XXXXX" } },
    { '0', { ".XXX.", "X...X", "X..XX", "X.X.X", "XX..X", "X...X", ".XXX." } },
    { '1', { "..X..", ".XX..", "..X..", "..X..", "..X..", "..X..", ".XXX." } },
    { '2', { ".XXX.", "X...X", "....X", "...X.", "..X..", ".X...", "XXXXX" } },
    { '3', { "XXXX.", "....X", "....X", ".XXX.", "....X", "....X", "XXXX." } },
    { '4', { "...X.", "..XX.", ".X.X.", "X..X.", "XXXXX", "...X.", "...X." } },
    { '5', { "XXXXX", "X....", "XXXX.", "....X", "....X", "X...X", ".XXX." } },
    { '6', { "..XX.", ".X...", "X....", "XXXX.", "X...X", "X...X", ".XXX." } },
    { '7', { "XXXXX", "....X", "...X.", "..X..", ".X...", ".X...", ".X..." } },
    { '8', { ".XXX.", "X...X", "X...X", ".XXX.", "X...X", "X...X", ".XXX." } },
    { '9', { ".XXX.", "X...X", "X...X", ".XXXX", "....X", "...X.", ".XX.." } },
    { '_', { ".....", ".....", ".....", ".....", ".....", ".....", "XXXXX" } },
    { '-', { ".....", ".....", ".....", "XXXXX", ".....", ".....", "....." } },
    { '.', { ".....", ".....", ".....", ".....", ".....", ".XX..", ".XX.." } }
};

const SGlyph*
FindGlyph( char character )
{
    char upper = character;
    if ( upper >= 'a' && upper <= 'z' )
        upper = (char) ( upper - 'a' + 'A' );
    for ( size_t i=0; i<sizeof( Glyphs ) / sizeof( Glyphs[ 0 ] ); ++i )
        if ( Glyphs[ i ].character == upper )
            return &Glyphs[ i ];
    return GUCEF_NULL;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CShowcaseView::SEntry::SEntry( void )
    : path(), label(), mesh(), entity( GUCEF_NULL ), node( GUCEF_NULL ), size( Ogre::Vector3::ZERO ), centre( Ogre::Vector3::ZERO ), bottom( 0.0f )
    , ownedMaterials(), textureNames(), groupName(), triangleCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CShowcaseView::CShowcaseView( CWorkbenchStage& stage )
    : m_stage( stage ), m_root( GUCEF_NULL ), m_entries(), m_layout(), m_sortedPaths(), m_sortedToEntry(), m_skipped( 0 ), m_arranged( false ), m_fader( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CShowcaseView::~CShowcaseView()
{GUCEF_TRACE;

    Unload();
}

/*-------------------------------------------------------------------------*/

/** Hides the letters of the names while they are too small to read (the camera is far away), before every drawing of the camera */
class CShowcaseView::CLabelFader : public Ogre::Camera::Listener
{
    public:

    CLabelFader( Ogre::Camera* camera, Ogre::ManualObject* labels, Float32 letterHeight )
        : m_camera( camera ), m_labels( labels ), m_letterHeight( letterHeight )
    {
        m_camera->addListener( this );
    }

    virtual ~CLabelFader() GUCEF_VIRTUAL_OVERRIDE
    {
    }

    Ogre::Camera* GetCamera( void ) const { return m_camera; }

    /** Takes the listener off the camera, when the camera is still there */
    void Detach( bool cameraAlive )
    {
        if ( cameraAlive && GUCEF_NULL != m_camera )
            m_camera->removeListener( this );
        m_camera = GUCEF_NULL;
    }

    virtual void cameraPreRenderScene( Ogre::Camera* camera ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_camera || camera != m_camera )
            return;
        const Ogre::Viewport* viewport = camera->getViewport();
        const UInt32 viewHeight = GUCEF_NULL != viewport ? (UInt32) viewport->getActualHeight() : 0;
        Float32 pixels = 0.0f;
        if ( Ogre::PT_ORTHOGRAPHIC == camera->getProjectionType() )
            pixels = camera->getOrthoWindowHeight() > 0.0f ? m_letterHeight / camera->getOrthoWindowHeight() * (Float32) viewHeight : 0.0f;
        else
        {
            const Ogre::AxisAlignedBox& box = m_labels->getWorldBoundingBox( true );
            const Float32 distance = box.isNull() ? 0.0f : ( box.getCenter() - camera->getDerivedPosition() ).length();
            pixels = GetLabelPixelHeight( m_letterHeight, distance, Ogre::Degree( camera->getFOVy() ).valueDegrees(), viewHeight );
        }
        const bool readable = IsLabelReadable( pixels );
        if ( readable != m_labels->getVisible() )
            m_labels->setVisible( readable );
    }

    private:

    Ogre::Camera* m_camera;
    Ogre::ManualObject* m_labels;
    Float32 m_letterHeight;
};

/*-------------------------------------------------------------------------*/

Float32
CShowcaseView::GetLabelHeight( Float32 pitch, UInt32 length, UInt32& keptLength )
{GUCEF_TRACE;

    // a letter with its gap is 6 dots wide and the height of a letter is 7 dots
    const Float32 widest = 0.95f * pitch;
    keptLength = length;
    Float32 height = 0.2f * pitch;
    if ( 0 == length )
        return height;
    const Float32 minimum = 0.08f * pitch;
    if ( 6.0f * (Float32) length * height / 7.0f > widest )
    {
        height = widest * 7.0f / ( 6.0f * (Float32) length );
        if ( height < minimum )
        {
            height = minimum;
            keptLength = (UInt32) ( widest * 7.0f / ( 6.0f * height ) );
            if ( keptLength < 1 )
                keptLength = 1;
        }
    }
    return height;
}

/*-------------------------------------------------------------------------*/

Float32
CShowcaseView::GetLabelPixelHeight( Float32 letterHeight, Float32 distance, Float32 fieldOfViewDegrees, UInt32 viewHeightInPixels )
{GUCEF_TRACE;

    if ( distance <= 1e-6f || fieldOfViewDegrees <= 0.0f )
        return 0.0f;
    const Float32 half = 0.5f * fieldOfViewDegrees * 3.14159265f / 180.0f;
    return letterHeight / ( 2.0f * distance * tanf( half ) ) * (Float32) viewHeightInPixels;
}

/*-------------------------------------------------------------------------*/

bool
CShowcaseView::IsLabelReadable( Float32 pixelHeight )
{GUCEF_TRACE;

    // the letters are seven dots high: below this each dot is smaller than a pixel and the text is a smear
    return pixelHeight >= 10.0f;
}

/*-------------------------------------------------------------------------*/

void
CShowcaseView::AddUprightTextLines( CWorkbenchStage::TLineVertexVector& lines, const CORE::CString& text, Float32 x, Float32 y, Float32 z, Float32 height, Float32 red, Float32 green, Float32 blue )
{GUCEF_TRACE;

    // the letters of the ground text, turned up: the rows run down from the top of the letters
    CWorkbenchStage::TLineVertexVector ground;
    AddTextLines( ground, text, x, 0.0f, height, red, green, blue );
    for ( size_t i=0; i<ground.size(); ++i )
    {
        CWorkbenchStage::SLineVertex v = ground[ i ];
        // ground z is the row (0 at the top of the letter), the height of the letter is the other way up
        v.y = y + height - ( v.z );
        v.z = z;
        lines.push_back( v );
    }
}

/*-------------------------------------------------------------------------*/

void
CShowcaseView::AddTextLines( CWorkbenchStage::TLineVertexVector& lines, const CORE::CString& text, Float32 x, Float32 z, Float32 height, Float32 red, Float32 green, Float32 blue )
{GUCEF_TRACE;

    const Float32 dot = height / 7.0f;
    Float32 penX = x;
    for ( UInt32 c=0; c<text.Length(); ++c )
    {
        const SGlyph* glyph = FindGlyph( text[ c ] );
        if ( GUCEF_NULL != glyph )
        {
            for ( UInt32 row=0; row<7; ++row )
            {
                const char* dots = glyph->rows[ row ];
                UInt32 column = 0;
                while ( column < 5 )
                {
                    if ( 'X' != dots[ column ] )
                    {
                        ++column;
                        continue;
                    }
                    UInt32 end = column;
                    while ( end + 1 < 5 && 'X' == dots[ end + 1 ] )
                        ++end;
                    // two lines per row make the stroke as thick as a dot
                    for ( UInt32 pass=0; pass<2; ++pass )
                    {
                        const Float32 lineZ = z + ( (Float32) row + 0.25f + 0.5f * (Float32) pass ) * dot;
                        CWorkbenchStage::SLineVertex a = { penX + (Float32) column * dot, 0.002f, lineZ, red, green, blue };
                        CWorkbenchStage::SLineVertex b = { penX + (Float32) ( end + 1 ) * dot, 0.002f, lineZ, red, green, blue };
                        lines.push_back( a );
                        lines.push_back( b );
                    }
                    column = end + 1;
                }
            }
        }
        penX += 6.0f * dot;
    }
}

/*-------------------------------------------------------------------------*/

bool
CShowcaseView::AddModel( const CORE::CString& groupName, const CORE::CString& meshName, const CORE::CString& path, CModelView::SPreparedMesh& prepared )
{GUCEF_TRACE;

    if ( !m_stage.IsInitialized() )
        return false;
    SEntry entry;
    try
    {
        GUCEF::set< Ogre::String > materialsBefore;
        {
            Ogre::ResourceManager::ResourceMapIterator materials = Ogre::MaterialManager::getSingleton().getResourceIterator();
            while ( materials.hasMoreElements() )
                materialsBefore.insert( materials.getNext()->getName() );
        }

        Ogre::MeshManager& meshes = Ogre::MeshManager::getSingleton();
        Ogre::MeshPtr existing = meshes.getByName( meshName.STL_String(), groupName.STL_String() );
        const bool preparedMesh = existing && prepared.mesh == existing;
        if ( existing && !preparedMesh && !existing->isManuallyLoaded() )
            meshes.remove( existing );
        entry.mesh = preparedMesh || ( existing && existing->isManuallyLoaded() ) ? existing :
                     Ogre::static_pointer_cast< Ogre::Mesh >( meshes.createOrRetrieve( meshName.STL_String(), groupName.STL_String(), false, GUCEF_NULL, GUCEF_NULL,
                                                                                         Ogre::HBU_GPU_ONLY, Ogre::HBU_GPU_ONLY, true, true ).first );
        if ( prepared.assimp.has_value() )
            entry.mesh->getUserObjectBindings().setUserAny( "_AssimpPrepared", prepared.assimp );
        entry.mesh->load();
        entry.mesh->getUserObjectBindings().eraseUserAny( "_AssimpPrepared" );
        prepared = CModelView::SPreparedMesh();

        SMeshStatistics statistics;
        CMeshInspector::GetStatistics( entry.mesh, statistics );
        if ( 0 == statistics.subMeshCount || 0 == statistics.vertexCount )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CShowcaseView: The model " << path << " has no geometry: it was not read" );
            meshes.remove( entry.mesh );
            ++m_skipped;
            return false;
        }
        entry.triangleCount = statistics.triangleCount;
        entry.textureNames = statistics.textureNames;

        Ogre::ResourceManager::ResourceMapIterator materialsAfter = Ogre::MaterialManager::getSingleton().getResourceIterator();
        while ( materialsAfter.hasMoreElements() )
        {
            const Ogre::String name = materialsAfter.getNext()->getName();
            if ( materialsBefore.find( name ) == materialsBefore.end() )
                entry.ownedMaterials.push_back( name );
        }

        if ( GUCEF_NULL == m_root )
            m_root = m_stage.GetContentNode()->createChildSceneNode();
        entry.entity = m_stage.GetSceneManager()->createEntity( entry.mesh );
        entry.node = m_root->createChildSceneNode();
        entry.node->attachObject( entry.entity );
        entry.groupName = groupName;
        entry.path = path;
        const Int32 slash = path.HasChar( '/', false );
        CORE::CString name = slash >= 0 ? path.SubstrFromRange( (UInt32) slash + 1, path.Length() ) : path;
        const Int32 dot = name.HasChar( '.', false );
        entry.label = dot > 0 ? name.SubstrFromRange( 0, (UInt32) dot ) : name;

        // the pictures that are not in the mount are taken off the materials
        CORE::CString::StringVector materialNames, materialGroups;
        for ( UInt32 i=0; i<(UInt32) entry.entity->getNumSubEntities(); ++i )
        {
            materialNames.push_back( entry.entity->getSubEntity( i )->getMaterialName() );
            materialGroups.push_back( entry.entity->getSubEntity( i )->getMaterial() ? entry.entity->getSubEntity( i )->getMaterial()->getGroup() : Ogre::String() );
        }
        CModelView::RemoveMissingTextures( materialNames, materialGroups, groupName );
        // the materials are set again, as the model view does after it: the shader generator then builds its state from what is left of them
        CWorkbenchMaterials::Ensure();
        for ( UInt32 i=0; i<(UInt32) entry.entity->getNumSubEntities(); ++i )
            if ( !materialNames[ i ].IsNULLOrEmpty() )
                entry.entity->getSubEntity( i )->setMaterialName( materialNames[ i ].STL_String(), materialGroups[ i ].IsNULLOrEmpty() ? Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME : materialGroups[ i ].STL_String() );

        // the bounds as the model stands: the formats made for printing are Z up
        Ogre::AxisAlignedBox bounds;
        if ( !CMeshInspector::GetGeometryBounds( entry.mesh, bounds ) )
            bounds = entry.mesh->getBounds();
        Ogre::Quaternion orientation = Ogre::Quaternion::IDENTITY;
        if ( CModelView::IsPrintFormat( CAssetTypeResolver::GetExtension( meshName ) ) )
            orientation = Ogre::Quaternion( Ogre::Degree( -90.0f ), Ogre::Vector3::UNIT_X );
        Ogre::AxisAlignedBox turned;
        if ( bounds.isFinite() && !bounds.isNull() )
        {
            const Ogre::AxisAlignedBox::Corners& corners = bounds.getAllCorners();
            for ( size_t i=0; i<8; ++i )
                turned.merge( orientation * corners[ i ] );
        }
        else
        {
            turned = Ogre::AxisAlignedBox( Ogre::Vector3( -0.5f, 0.0f, -0.5f ), Ogre::Vector3( 0.5f, 1.0f, 0.5f ) );
        }
        entry.node->setOrientation( orientation );
        entry.size = turned.getSize();
        entry.centre = turned.getCenter();
        entry.bottom = turned.getMinimum().y;
        m_entries.push_back( entry );
        m_arranged = false;
        m_stage.MarkDirty();
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CShowcaseView: The model " << path << " cannot be loaded: " << e.getDescription() );
        DestroyEntry( entry );
        CModelView::SPreparedMesh discarded = prepared;
        CModelView::DiscardPrepared( discarded );
        prepared = CModelView::SPreparedMesh();
        ++m_skipped;
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CShowcaseView::Arrange( const CPackShowcaseLayout::SOptions& options )
{GUCEF_TRACE;

    if ( m_entries.empty() )
        return false;
    CPackShowcaseLayout::TItemVector items;
    for ( size_t i=0; i<m_entries.size(); ++i )
    {
        CPackShowcaseLayout::SItem item;
        item.path = m_entries[ i ].path;
        item.group = CPackShowcaseLayout::GetGroupOfPath( m_entries[ i ].path );
        item.sizeX = m_entries[ i ].size.x;
        item.sizeY = m_entries[ i ].size.y;
        item.sizeZ = m_entries[ i ].size.z;
        items.push_back( item );
    }
    if ( !m_layout.Compute( items, options ) )
        return false;
    m_sortedPaths.clear();
    m_sortedToEntry.clear();
    CWorkbenchStage::TLineVertexVector labels;
    Float32 letterHeight = 0.2f * m_layout.GetPitch();
    const CPackShowcaseLayout::TCellVector& cells = m_layout.GetCells();
    const Float32 pitch = m_layout.GetPitch();
    for ( size_t c=0; c<cells.size(); ++c )
    {
        const CPackShowcaseLayout::SCell& cell = cells[ c ];
        m_sortedPaths.push_back( items[ cell.item ].path );
        UInt32 entryIndex = 0;
        for ( size_t e=0; e<m_entries.size(); ++e )
            if ( m_entries[ e ].path == items[ cell.item ].path )
                entryIndex = (UInt32) e;
        m_sortedToEntry.push_back( entryIndex );
        SEntry& entry = m_entries[ entryIndex ];
        entry.node->setScale( cell.scale, cell.scale, cell.scale );
        entry.node->setPosition( cell.x - entry.centre.x * cell.scale, -entry.bottom * cell.scale, cell.z - entry.centre.z * cell.scale );

        // the name standing in front of the model, as big as its cell allows (see GetLabelHeight()); a long name is cut where its letters would be too small
        CORE::CString text = entry.label;
        UInt32 kept = text.Length();
        const Float32 height = GetLabelHeight( pitch, text.Length(), kept );
        if ( kept < text.Length() )
            text = text.SubstrFromRange( 0, kept );
        letterHeight = height;
        const Float32 width = 6.0f * (Float32) text.Length() * height / 7.0f;
        const Float32 footprint = ( entry.size.x > entry.size.z ? entry.size.x : entry.size.z ) * cell.scale;
        AddUprightTextLines( labels, text, cell.x - 0.5f * width, 0.0f, cell.z + 0.5f * footprint + 0.1f * height, height, 0.85f, 0.88f, 0.95f );
    }
    Ogre::ManualObject* object = m_stage.GetLineObject( LabelLineObjectName );
    if ( GUCEF_NULL != object )
    {
        CWorkbenchStage::FillLineObject( *object, labels );
        // the names are drawn only while their letters can be read: the camera of the stage tells before every drawing
        RemoveFader();
        if ( GUCEF_NULL != m_stage.GetCamera() )
            m_fader = GUCEF_NEW CLabelFader( m_stage.GetCamera(), object, letterHeight );
    }
    m_arranged = true;
    m_stage.SetGridUnit( pitch * 0.25f );
    m_stage.MarkDirty();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CShowcaseView::DestroyEntry( SEntry& entry )
{GUCEF_TRACE;

    try
    {
        if ( GUCEF_NULL != entry.entity && GUCEF_NULL != entry.node )
            entry.node->detachObject( entry.entity );
        if ( GUCEF_NULL != entry.entity )
            m_stage.GetSceneManager()->destroyEntity( entry.entity );
        if ( GUCEF_NULL != entry.node && GUCEF_NULL != m_root )
            m_root->removeAndDestroyChild( entry.node );
        entry.entity = GUCEF_NULL;
        entry.node = GUCEF_NULL;
        if ( entry.mesh && !entry.mesh->isManuallyLoaded() )
        {
            const Ogre::String skeletonName = entry.mesh->getSkeletonName();
            Ogre::MeshManager::getSingleton().remove( entry.mesh );
            if ( !skeletonName.empty() && Ogre::SkeletonManager::getSingleton().resourceExists( skeletonName, entry.groupName.STL_String() ) )
                Ogre::SkeletonManager::getSingleton().remove( skeletonName, entry.groupName.STL_String() );
        }
        for ( size_t i=0; i<entry.ownedMaterials.size(); ++i )
            if ( Ogre::MaterialManager::getSingleton().resourceExists( entry.ownedMaterials[ i ].STL_String() ) )
                Ogre::MaterialManager::getSingleton().remove( entry.ownedMaterials[ i ].STL_String() );
        for ( size_t i=0; i<entry.textureNames.size(); ++i )
        {
            Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName( entry.textureNames[ i ].STL_String(), entry.groupName.STL_String() );
            if ( texture )
                texture->unload();
        }
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CShowcaseView: A model cannot be taken out of the stage: " << e.getDescription() );
    }
    entry.mesh.reset();
}

/*-------------------------------------------------------------------------*/

void
CShowcaseView::RemoveFader( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_fader )
        return;
    m_fader->Detach( GUCEF_NULL != Ogre::Root::getSingletonPtr() && m_stage.IsInitialized() && m_stage.GetCamera() == m_fader->GetCamera() );
    GUCEF_DELETE m_fader;
    m_fader = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CShowcaseView::Unload( void )
{GUCEF_TRACE;

    RemoveFader();

    if ( GUCEF_NULL != Ogre::Root::getSingletonPtr() && m_stage.IsInitialized() )
    {
        try
        {
            m_stage.ClearLineObjects();
            for ( size_t i=0; i<m_entries.size(); ++i )
                DestroyEntry( m_entries[ i ] );
            if ( GUCEF_NULL != m_root )
            {
                m_stage.GetContentNode()->removeAndDestroyChild( m_root );
                m_stage.MarkDirty();
            }
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CShowcaseView: The showcase cannot be taken out of the stage: " << e.getDescription() );
        }
    }
    m_root = GUCEF_NULL;
    m_entries.clear();
    m_sortedPaths.clear();
    m_sortedToEntry.clear();
    m_layout = CPackShowcaseLayout();
    m_skipped = 0;
    m_arranged = false;
}

/*-------------------------------------------------------------------------*/

bool CShowcaseView::IsLoaded( void ) const { return !m_entries.empty(); }
UInt32 CShowcaseView::GetModelCount( void ) const { return (UInt32) m_entries.size(); }
UInt32 CShowcaseView::GetSkippedCount( void ) const { return m_skipped; }
const CPackShowcaseLayout& CShowcaseView::GetLayout( void ) const { return m_layout; }

/*-------------------------------------------------------------------------*/

const CORE::CString&
CShowcaseView::GetModelPath( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_entries.size() ? m_entries[ index ].path : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

Ogre::SceneNode*
CShowcaseView::GetModelNode( UInt32 index ) const
{GUCEF_TRACE;

    return index < m_entries.size() ? m_entries[ index ].node : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CShowcaseView::GetModelCell( UInt32 index, CPackShowcaseLayout::SCell& cell ) const
{GUCEF_TRACE;

    if ( !m_arranged )
        return false;
    for ( size_t i=0; i<m_sortedToEntry.size(); ++i )
    {
        if ( m_sortedToEntry[ i ] == index )
        {
            cell = m_layout.GetCells()[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

Int32
CShowcaseView::FindModelAtGroundPoint( Float32 x, Float32 z ) const
{GUCEF_TRACE;

    if ( !m_arranged )
        return -1;
    const Float32 half = 0.5f * m_layout.GetPitch();
    const CPackShowcaseLayout::TCellVector& cells = m_layout.GetCells();
    for ( size_t i=0; i<cells.size(); ++i )
        if ( fabsf( x - cells[ i ].x ) <= half && fabsf( z - cells[ i ].z ) <= half )
            return (Int32) m_sortedToEntry[ i ];
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CShowcaseView::GetBoundingSphere( COrbitController::SVector3& center, Float32& radius ) const
{GUCEF_TRACE;

    center = COrbitController::SVector3();
    radius = 1.0f;
    if ( !m_arranged )
        return;
    Float32 x = 0.0f, y = 0.0f, z = 0.0f;
    m_layout.GetBoundingSphere( x, y, z, radius );
    center = COrbitController::SVector3( x, y, z );
}

/*-------------------------------------------------------------------------*/

void
CShowcaseView::WriteInspectorData( CORE::CDataNode& root ) const
{GUCEF_TRACE;

    CORE::CDataNode* node = root.AddChild( "Showcase" );
    node->SetAttribute( "Models", (UInt32) m_entries.size() );
    node->SetAttribute( "Skipped", m_skipped );
    node->SetAttribute( "Folders", m_layout.GetGroupCount() );
    node->SetAttribute( "Rows", m_layout.GetRowCount() );
    UInt32 triangles = 0;
    for ( size_t i=0; i<m_entries.size(); ++i )
        triangles += m_entries[ i ].triangleCount;
    node->SetAttribute( "Triangles", triangles );

    // the names in the order of the arrangement: the letters in the view are small when many models are in it
    CORE::CDataNode* names = node->AddChild( "Names" );
    for ( size_t i=0; i<m_sortedToEntry.size() && i<m_entries.size(); ++i )
        names->SetAttribute( CORE::UInt32ToString( (UInt32) i + 1 ), m_entries[ m_sortedToEntry[ i ] ].label );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
