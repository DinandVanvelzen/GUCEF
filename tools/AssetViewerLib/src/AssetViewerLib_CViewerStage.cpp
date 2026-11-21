/*
 *  AssetViewerLib: a scene, a camera and a render texture that an asset is shown in
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

#ifndef PLUGINGLUE_OGRE_COGREGLOBAL_H
#include "pluginglueOGRE_COgreGlobal.h"
#define PLUGINGLUE_OGRE_COGREGLOBAL_H
#endif /* PLUGINGLUE_OGRE_COGREGLOBAL_H ? */

#ifndef PLUGINGLUE_OGRE_COGREROOTSETUP_H
#include "pluginglueOGRE_COgreRootSetup.h"
#define PLUGINGLUE_OGRE_COGREROOTSETUP_H
#endif /* PLUGINGLUE_OGRE_COGREROOTSETUP_H ? */

#ifndef PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H
#include "pluginglueOGRE_COgreShaderGeneratorSceneTracker.h"
#define PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H
#endif /* PLUGINGLUE_OGRE_COGRESHADERGENERATORSCENETRACKER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETVIEWERLIB_CVIEWERMATERIALS_H
#include "AssetViewerLib_CViewerMaterials.h"
#define ASSETVIEWERLIB_CVIEWERMATERIALS_H
#endif /* ASSETVIEWERLIB_CVIEWERMATERIALS_H ? */

#include "AssetViewerLib_CViewerStage.h"

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

const UInt32 GridHalfCells = 8;

struct SLightSetting
{
    Float32 key;
    Float32 fill;
    Float32 rim;
    Float32 ambient;
};

/** Per preset: the strength of the key, fill and rim light and the ambient light */
const SLightSetting LightSettings[ CViewerStage::LIGHTPRESET_COUNT ] =
{
    { 0.95f, 0.30f, 0.00f, 0.22f },     // studio
    { 0.85f, 0.35f, 0.60f, 0.12f },     // three point
    { 0.00f, 0.00f, 0.00f, 1.00f },     // flat: ambient light only
    { 0.35f, 0.00f, 0.00f, 0.08f }      // dim
};

void
AddLine( CViewerStage::TLineVertexVector& lines, Float32 x1, Float32 y1, Float32 z1, Float32 x2, Float32 y2, Float32 z2, Float32 r, Float32 g, Float32 b )
{
    CViewerStage::SLineVertex a = { x1, y1, z1, r, g, b };
    CViewerStage::SLineVertex c = { x2, y2, z2, r, g, b };
    lines.push_back( a );
    lines.push_back( c );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CViewerStage::CViewerStage( const CORE::CString& name )
    : m_name( name )
    , m_sceneManager( GUCEF_NULL )
    , m_camera( GUCEF_NULL )
    , m_cameraNode( GUCEF_NULL )
    , m_contentNode( GUCEF_NULL )
    , m_grid( GUCEF_NULL )
    , m_axes( GUCEF_NULL )
    , m_lineObjects()
    , m_texture()
    , m_renderTarget( GUCEF_NULL )
    , m_viewport( GUCEF_NULL )
    , m_width( 0 )
    , m_height( 0 )
    , m_lightPreset( LIGHTPRESET_STUDIO )
    , m_gridUnit( 0.2f )
    , m_gridVisible( true )
    , m_axesVisible( true )
    , m_wireframe( false )
    , m_dirty( true )
    , m_textureCounter( 0 )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<3; ++i )
    {
        m_lights[ i ] = GUCEF_NULL;
        m_lightNodes[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CViewerStage::~CViewerStage()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::Initialize( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( IsInitialized() )
        return true;
    if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() || !CViewerMaterials::Ensure() )
        return false;

    try
    {
        Ogre::Root& root = Ogre::Root::getSingleton();
        m_sceneManager = root.createSceneManager( "DefaultSceneManager", m_name.STL_String() );

        // the shader generator knows a scene manager once it is tracked, which the tracker does at the next frame of the Root:
        // a stage that is drawn before that frame (a render texture update) would draw without an active scene manager there
        PLUGINGLUE::OGRE::COgreGlobal* ogreGlobal = PLUGINGLUE::OGRE::COgreGlobal::Instance();
        PLUGINGLUE::OGRE::COgreShaderGeneratorSceneTracker* tracker = GUCEF_NULL == ogreGlobal ? GUCEF_NULL : ogreGlobal->GetRootSetup().GetShaderGeneratorSceneTracker();
        if ( GUCEF_NULL != tracker )
            tracker->AddNewSceneManagers();
        m_camera = m_sceneManager->createCamera( "Camera" );
        m_cameraNode = m_sceneManager->getRootSceneNode()->createChildSceneNode();
        m_cameraNode->attachObject( m_camera );
        // the camera does not roll: the vertical axis of the world stays vertical on the picture
        m_cameraNode->setFixedYawAxis( true, Ogre::Vector3::UNIT_Y );
        m_contentNode = m_sceneManager->getRootSceneNode()->createChildSceneNode();

        // the lights hang on the camera: key from the upper left, fill from the lower right, rim from behind
        const char* const lightNames[ 3 ] = { "Key", "Fill", "Rim" };
        const Ogre::Vector3 directions[ 3 ] = { Ogre::Vector3( 0.5f, -0.7f, -1.0f ), Ogre::Vector3( -0.6f, 0.3f, -1.0f ), Ogre::Vector3( 0.2f, -0.2f, 1.0f ) };
        for ( UInt32 i=0; i<3; ++i )
        {
            m_lights[ i ] = m_sceneManager->createLight( lightNames[ i ], Ogre::Light::LT_DIRECTIONAL );
            m_lightNodes[ i ] = m_cameraNode->createChildSceneNode();
            m_lightNodes[ i ]->attachObject( m_lights[ i ] );
            m_lightNodes[ i ]->setDirection( directions[ i ], Ogre::Node::TS_PARENT );
        }

        m_grid = m_sceneManager->createManualObject( "Grid" );
        m_grid->setDynamic( true );
        m_axes = m_sceneManager->createManualObject( "Axes" );
        m_axes->setDynamic( true );
        m_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject( m_grid );
        m_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject( m_axes );
        RebuildGrid();

        if ( !MakeRenderTexture( width, height ) )
        {
            Shutdown();
            return false;
        }
        SetLightPreset( m_lightPreset );
        SetBackgroundColour( 0.17f, 0.18f, 0.21f );
        m_dirty = true;
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CViewerStage: The stage " << m_name << " cannot be made: " << e.getFullDescription() );
        Shutdown();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::Shutdown( void )
{GUCEF_TRACE;

    ReleaseRenderTexture();
    if ( GUCEF_NULL != m_sceneManager && GUCEF_NULL != Ogre::Root::getSingletonPtr() )
    {
        try
        {
            Ogre::Root::getSingleton().destroySceneManager( m_sceneManager );
        }
        catch ( const Ogre::Exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CViewerStage: The scene manager of " << m_name << " cannot be destroyed: " << e.getFullDescription() );
        }
    }
    m_sceneManager = GUCEF_NULL;
    m_camera = GUCEF_NULL;
    m_cameraNode = GUCEF_NULL;
    m_contentNode = GUCEF_NULL;
    m_grid = GUCEF_NULL;
    m_axes = GUCEF_NULL;
    m_lineObjects.clear();
    for ( UInt32 i=0; i<3; ++i )
    {
        m_lights[ i ] = GUCEF_NULL;
        m_lightNodes[ i ] = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::IsInitialized( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_sceneManager && GUCEF_NULL != m_renderTarget;
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::MakeRenderTexture( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( width < 1 )
        width = 1;
    if ( height < 1 )
        height = 1;

    // a new name for every texture: the old one may still be known to the render system for a frame
    const CORE::CString textureName = "AssetViewer/Stage/" + m_name + "/" + CORE::UInt32ToString( ++m_textureCounter );
    m_texture = Ogre::TextureManager::getSingleton().createManual( textureName.STL_String(), Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D,
                                                                  width, height, 0, Ogre::PF_BYTE_RGBA, Ogre::TU_RENDERTARGET );
    if ( !m_texture )
        return false;
    m_renderTarget = m_texture->getBuffer()->getRenderTarget();
    if ( GUCEF_NULL == m_renderTarget )
        return false;
    m_renderTarget->setAutoUpdated( false );
    // a depth buffer of its own: the one of an earlier texture of another size is not shared with this one
    static Ogre::uint16 depthPools = 100;
    m_renderTarget->setDepthBufferPool( ++depthPools );
    m_viewport = m_renderTarget->addViewport( m_camera );
    m_viewport->setClearEveryFrame( true );
    m_viewport->setOverlaysEnabled( false );
    m_viewport->setBackgroundColour( Ogre::ColourValue( 0.17f, 0.18f, 0.21f, 1.0f ) );
    m_width = width;
    m_height = height;
    m_camera->setAspectRatio( (Float32) width / (Float32) height );
    m_dirty = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::ReleaseRenderTexture( void )
{GUCEF_TRACE;

    ReleaseRenderTexture( m_texture, m_renderTarget );
    m_viewport = GUCEF_NULL;
    m_renderTarget = GUCEF_NULL;
    m_texture.reset();
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::ReleaseRenderTexture( Ogre::TexturePtr& texture, Ogre::RenderTarget* renderTarget )
{GUCEF_TRACE;

    if ( GUCEF_NULL != renderTarget )
        renderTarget->removeAllViewports();
    if ( texture && GUCEF_NULL != Ogre::TextureManager::getSingletonPtr() )
        Ogre::TextureManager::getSingleton().remove( texture );
    texture.reset();
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::Resize( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( !IsInitialized() )
        return false;
    if ( width == m_width && height == m_height )
        return true;

    // the new texture and its viewport are made before the old ones go, so a failure leaves the stage as it was
    Ogre::TexturePtr oldTexture = m_texture;
    Ogre::RenderTarget* oldTarget = m_renderTarget;
    const Ogre::ColourValue background = m_viewport->getBackgroundColour();
    try
    {
        if ( !MakeRenderTexture( width, height ) )
        {
            m_texture = oldTexture;
            m_renderTarget = oldTarget;
            return false;
        }
        m_viewport->setBackgroundColour( background );
        ReleaseRenderTexture( oldTexture, oldTarget );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CViewerStage: The texture of " << m_name << " cannot be resized: " << e.getFullDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CViewerStage::GetWidth( void ) const
{GUCEF_TRACE;

    return m_width;
}

/*-------------------------------------------------------------------------*/

UInt32
CViewerStage::GetHeight( void ) const
{GUCEF_TRACE;

    return m_height;
}

/*-------------------------------------------------------------------------*/

Ogre::SceneManager*
CViewerStage::GetSceneManager( void ) const
{GUCEF_TRACE;

    return m_sceneManager;
}

/*-------------------------------------------------------------------------*/

Ogre::Camera*
CViewerStage::GetCamera( void ) const
{GUCEF_TRACE;

    return m_camera;
}

/*-------------------------------------------------------------------------*/

Ogre::SceneNode*
CViewerStage::GetContentNode( void ) const
{GUCEF_TRACE;

    return m_contentNode;
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
CViewerStage::GetRenderTexture( void ) const
{GUCEF_TRACE;

    return m_texture;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CViewerStage::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::ApplyController( const COrbitController& controller )
{GUCEF_TRACE;

    if ( !IsInitialized() )
        return;
    const COrbitController::SVector3 eye = controller.GetEye();
    const COrbitController::SVector3& target = controller.GetTarget();
    m_cameraNode->setPosition( eye.x, eye.y, eye.z );
    m_cameraNode->lookAt( Ogre::Vector3( target.x, target.y, target.z ), Ogre::Node::TS_WORLD );
    m_camera->setFOVy( Ogre::Degree( controller.GetFieldOfView() ) );
    m_camera->setNearClipDistance( controller.GetNearClipDistance() );
    m_camera->setFarClipDistance( controller.GetFarClipDistance() );
    m_camera->setAspectRatio( (Float32) m_width / (Float32) ( m_height > 0 ? m_height : 1 ) );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::SetBackgroundColour( Float32 red, Float32 green, Float32 blue )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_viewport )
        m_viewport->setBackgroundColour( Ogre::ColourValue( red, green, blue, 1.0f ) );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::SetLightPreset( ELightPreset preset )
{GUCEF_TRACE;

    if ( (Int32) preset < 0 || (Int32) preset >= (Int32) LIGHTPRESET_COUNT )
        return;
    m_lightPreset = preset;
    if ( GUCEF_NULL == m_sceneManager )
        return;
    const SLightSetting& setting = LightSettings[ preset ];
    const Float32 strengths[ 3 ] = { setting.key, setting.fill, setting.rim };
    const Ogre::ColourValue colours[ 3 ] = { Ogre::ColourValue( 1.0f, 0.97f, 0.92f ), Ogre::ColourValue( 0.78f, 0.86f, 1.0f ), Ogre::ColourValue( 1.0f, 1.0f, 1.0f ) };
    for ( UInt32 i=0; i<3; ++i )
    {
        m_lights[ i ]->setVisible( strengths[ i ] > 0.0f );
        m_lights[ i ]->setDiffuseColour( colours[ i ] * strengths[ i ] );
        m_lights[ i ]->setSpecularColour( colours[ i ] * ( 0.35f * strengths[ i ] ) );
    }
    m_sceneManager->setAmbientLight( Ogre::ColourValue( setting.ambient, setting.ambient, setting.ambient, 1.0f ) );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

CViewerStage::ELightPreset
CViewerStage::GetLightPreset( void ) const
{GUCEF_TRACE;

    return m_lightPreset;
}

/*-------------------------------------------------------------------------*/

const char*
CViewerStage::GetLightPresetName( ELightPreset preset )
{GUCEF_TRACE;

    switch ( preset )
    {
        case LIGHTPRESET_STUDIO:        return "Studio";
        case LIGHTPRESET_THREE_POINT:   return "Three point";
        case LIGHTPRESET_FLAT:          return "Flat";
        default:                        return "Dim";
    }
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::SetGridVisible( bool visible )
{GUCEF_TRACE;

    m_gridVisible = visible;
    if ( GUCEF_NULL != m_grid )
        m_grid->setVisible( visible );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::IsGridVisible( void ) const
{GUCEF_TRACE;

    return m_gridVisible;
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::SetAxesVisible( bool visible )
{GUCEF_TRACE;

    m_axesVisible = visible;
    if ( GUCEF_NULL != m_axes )
        m_axes->setVisible( visible );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::IsAxesVisible( void ) const
{GUCEF_TRACE;

    return m_axesVisible;
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::SetGridUnit( Float32 unit )
{GUCEF_TRACE;

    if ( unit <= 0.0f )
        return;
    m_gridUnit = unit;
    RebuildGrid();
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

Float32
CViewerStage::GetGridUnit( void ) const
{GUCEF_TRACE;

    return m_gridUnit;
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::RebuildGrid( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_grid || GUCEF_NULL == m_axes )
        return;

    TLineVertexVector gridLines;
    const Float32 extent = m_gridUnit * (Float32) GridHalfCells;
    for ( Int32 i=-(Int32) GridHalfCells; i<=(Int32) GridHalfCells; ++i )
    {
        if ( 0 == i )
            continue;   // the axes take the middle lines
        const Float32 at = m_gridUnit * (Float32) i;
        const Float32 grey = 0 == ( i % 5 ) ? 0.48f : 0.30f;
        AddLine( gridLines, -extent, 0.0f, at, extent, 0.0f, at, grey, grey, grey );
        AddLine( gridLines, at, 0.0f, -extent, at, 0.0f, extent, grey, grey, grey );
    }
    FillLineObject( *m_grid, gridLines );

    TLineVertexVector axisLines;
    AddLine( axisLines, -extent, 0.0f, 0.0f, extent, 0.0f, 0.0f, 0.75f, 0.2f, 0.2f );
    AddLine( axisLines, 0.0f, 0.0f, -extent, 0.0f, 0.0f, extent, 0.2f, 0.35f, 0.8f );
    AddLine( axisLines, 0.0f, 0.0f, 0.0f, 0.0f, extent * 0.5f, 0.0f, 0.25f, 0.8f, 0.25f );
    FillLineObject( *m_axes, axisLines );
    m_grid->setVisible( m_gridVisible );
    m_axes->setVisible( m_axesVisible );
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::SetWireframe( bool wireframe )
{GUCEF_TRACE;

    m_wireframe = wireframe;
    if ( GUCEF_NULL != m_camera )
        m_camera->setPolygonMode( wireframe ? Ogre::PM_WIREFRAME : Ogre::PM_SOLID );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::IsWireframe( void ) const
{GUCEF_TRACE;

    return m_wireframe;
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::Render( void )
{GUCEF_TRACE;

    if ( !IsInitialized() )
        return false;
    try
    {
        m_renderTarget->update();
        m_dirty = false;
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CViewerStage: The stage " << m_name << " cannot be drawn: " << e.getFullDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::IsDirty( void ) const
{GUCEF_TRACE;

    return m_dirty;
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::MarkDirty( void )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CViewerStage::ReadPixels( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( !IsInitialized() )
        return false;
    try
    {
        pixels = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, m_width, m_height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
        Ogre::PixelBox box( m_width, m_height, 1, Ogre::PF_BYTE_RGBA, pixels->GetDataPtr() );
        m_renderTarget->copyContentsToMemory( Ogre::Box( 0, 0, m_width, m_height ), box );
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CViewerStage: The picture of " << m_name << " cannot be read back: " << e.getFullDescription() );
        pixels.Unlink();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

Ogre::ManualObject*
CViewerStage::GetLineObject( const CORE::CString& name )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sceneManager )
        return GUCEF_NULL;
    GUCEF::map< CORE::CString, Ogre::ManualObject* >::iterator i = m_lineObjects.find( name );
    if ( i != m_lineObjects.end() )
        return i->second;
    try
    {
        Ogre::ManualObject* object = m_sceneManager->createManualObject( ( "Lines/" + name ).STL_String() );
        object->setDynamic( true );
        m_contentNode->createChildSceneNode()->attachObject( object );
        m_lineObjects[ name ] = object;
        return object;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CViewerStage: The line object " << name << " cannot be made: " << e.getFullDescription() );
        return GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::ClearLineObjects( void )
{GUCEF_TRACE;

    GUCEF::map< CORE::CString, Ogre::ManualObject* >::iterator i = m_lineObjects.begin();
    while ( i != m_lineObjects.end() )
    {
        i->second->clear();
        ++i;
    }
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::FillLineObject( Ogre::ManualObject& object, const TLineVertexVector& vertices )
{GUCEF_TRACE;

    object.clear();
    if ( vertices.empty() )
        return;
    object.begin( CViewerMaterials::LinesMaterialName.STL_String(), Ogre::RenderOperation::OT_LINE_LIST );
    for ( size_t i=0; i<vertices.size(); ++i )
    {
        object.position( vertices[ i ].x, vertices[ i ].y, vertices[ i ].z );
        object.colour( vertices[ i ].r, vertices[ i ].g, vertices[ i ].b, 1.0f );
    }
    object.end();
}

/*-------------------------------------------------------------------------*/

void
CViewerStage::AddBoxLines( TLineVertexVector& lines, const Ogre::AxisAlignedBox& box, Float32 red, Float32 green, Float32 blue )
{GUCEF_TRACE;

    if ( !box.isFinite() )
        return;
    const Ogre::Vector3& lo = box.getMinimum();
    const Ogre::Vector3& hi = box.getMaximum();
    const Float32 x[ 2 ] = { lo.x, hi.x };
    const Float32 y[ 2 ] = { lo.y, hi.y };
    const Float32 z[ 2 ] = { lo.z, hi.z };
    for ( UInt32 a=0; a<2; ++a )
    {
        for ( UInt32 b=0; b<2; ++b )
        {
            AddLine( lines, x[ 0 ], y[ a ], z[ b ], x[ 1 ], y[ a ], z[ b ], red, green, blue );
            AddLine( lines, x[ a ], y[ 0 ], z[ b ], x[ a ], y[ 1 ], z[ b ], red, green, blue );
            AddLine( lines, x[ a ], y[ b ], z[ 0 ], x[ a ], y[ b ], z[ 1 ], red, green, blue );
        }
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
